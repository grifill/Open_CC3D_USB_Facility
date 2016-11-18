#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <math.h>
#include <QMessageBox>

#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    about(NULL)
{
    ui->setupUi(this);
    bZeroCalibration = false;
    ui->refreshPorts_button->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    about.setAppName("РК 2.2 | ДГИ");
    measure.start();

    pokeTimer.start(5);
    connect(&pokeTimer, SIGNAL(timeout()), SLOT(onPokeTimer()));

    speedTimer.start(500);
    connect(&speedTimer, SIGNAL(timeout()), SLOT(onSpeedTimer()));

    connect(&cg, SIGNAL(newDump(QString)), SLOT(onCgLog(QString)));

    connect(&srv, SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(&ptsrv, SIGNAL(newConnection()), SLOT(onNewPtConnection()));

    on_refreshPorts_button_clicked();

    ui->widget->setAccels(ui->x_sb->value(), ui->y_sb->value(), ui->z_sb->value());
    ui->widget->setPlateRotations(ui->pitch_sb->value(), ui->roll_sb->value());
    ui->pointWidget->addTail(0, QPoint(0, 0), QPoint(0, 0));
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    closeServer();
    cg.disconnectFromDevice();
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *)
{
    about.close();
}
//------------------------------------------------------------------------------
void MainWindow::onPokeTimer()
{
    QTime t1;
    t1.start();

    AccPack data;
    Rotation rot;
    memset(&data, 0, sizeof(data));
    memset(&rot, 0, sizeof(rot));
    if(cg.isConnected())
    {
        cg.readPendingData();
        ui->comSpeed_label->setText(QString::number(cg.getSpeed())
                                    + QString::fromUtf8(" Б/с"));
        cg.getData(&data);
        cg.getRotation(&rot);

        ui->x_sb->setValue(data.ax);
        ui->y_sb->setValue(data.ay);
        ui->z_sb->setValue(data.az);
        ui->gx_sb->setValue(data.gx);
        ui->gy_sb->setValue(data.gy);
        ui->gz_sb->setValue(data.gz);
        ui->T_sb->setValue((float)data.temp/256);
    }
    else
    {
        data.ax = ui->x_sb->value();
        data.ay = ui->y_sb->value();
        data.az = ui->z_sb->value();
        data.gx = ui->gx_sb->value();
        data.gy = ui->gy_sb->value();
        data.gz = ui->gz_sb->value();
        data.temp = ui->T_sb->value();
    }

//    qDebug() << "Timer tick: " << t1.elapsed();

    ui->widget->setAccels(data.ax, data.ay, data.az);

    double pitch, roll;
    double normx = sqrt((double)data.ax*data.ax +
                        (double)data.az*data.az);
    double normy = sqrt((double)data.ay*data.ay +
                        (double)data.az*data.az);
    if(normx == 0)
    {
        pitch = 0;
    }
    else
    {
        pitch = asin((double)data.ax/normx)*180.0/M_PI;
        if(data.az < 0)
            pitch = 180-pitch;
    }
    if(normy == 0)
    {
        roll = 0;
    }
    else
    {
        roll = asin((double)data.ay/normy)*180.0/M_PI;
        if(data.az < 0)
            roll = 180-roll;
    }
    ui->pitch_sb->setValue(pitch);
    ui->roll_sb->setValue(roll);

    //ui->widget->setPlateRotations(pitch, roll);
    ui->widget->setPlateRotations(-rot.ry, rot.rx, rot.rz);

    // Проверяем состояние калибровки
    if(bCoeffCalibration)
    {
        if(calibTime.elapsed() > 5000)
        {
            float k = cg.getAngleMeasure();
            cg.setGyroCalibration(k);
            QMessageBox::information(
                this,
                QString::fromUtf8("Калибровка гироскопа"),
                QString::fromUtf8("Калибровка угла завершена"));
            bCoeffCalibration = false;
        }
    }
    if(bZeroCalibration)
    {
        if(calibTime.elapsed() > 5000)
        {
            bZeroCalibration = false;
            gyroDrift = cg.getDriftMeasure();
            cg.setGyroCalibration(gyroDrift);
            QMessageBox::information(
                this,
                QString::fromUtf8("Калибровка гироскопа"),
                QString::fromUtf8("Калибровка нуля завершена"));
        }

    }

    QPoint vel = QPoint(0, 0);

    if( abs(data.ax)>100 || abs(data.ay)>100 )
    {
        if(abs(data.ax) > abs(data.ay))
        {
            vel.setX(data.ax / abs(data.ax));
        }
        else
        {
            vel.setY(data.ay / abs(data.ay));
        }
    }
    ui->pointWidget->updateVelocity(0, vel);
    QPoint pt = ui->pointWidget->position(0);
    ui->px_sb->setValue(pt.x());
    ui->py_sb->setValue(pt.y());

    // Рассылаем клиентам первого сервера ускорение
    for(int a=0; a<clients.size(); a++)
    {
        QTcpSocket* s = clients[a];
        struct
        {
            double x;
            double y;
            double z;
        }acc;
        acc.x = data.ax;
        acc.y = data.ay;
        acc.z = data.az;

        s->write((char*)&acc, sizeof(acc));
        sent += sizeof(acc);
    }


    QList<int>& ids = ui->pointWidget->getIDs();

    int packsize = 3*ids.size()+1;
    int* pack = new int[packsize];
    for(int a=0; a<ids.size(); a++)
    {
        pack[3*a] = ids[a];
        QPoint pt = ui->pointWidget->position(ids[a]);
        pack[3*a+1] = pt.x();
        pack[3*a+2] = pt.y();
    }
    pack[3*ids.size()] = -1;

    // Рассылаем клиентам второго сервера координаты
    for(int a=0; a<ptclients.size(); a++)
    {
        QTcpSocket* s = ptclients[a];
        s->write((char*)pack, packsize*sizeof(int));
        sent += packsize*sizeof(int);

    }

}
//------------------------------------------------------------------------------
void MainWindow::onSpeedTimer()
{
    int ms = measure.elapsed();
    double speed = 1.0e3 * (sent - lastSent) / ms;
    ui->statusBar->showMessage(
        QString::fromUtf8("Отправлено ") + QString::number(sent - lastSent)
        + QString::fromUtf8(" байт для ") + QString::number(clients.size())
        + QString::fromUtf8(" клиентов за ") + QString::number(ms)
        + QString::fromUtf8("мс, скорость: ")
        + QString::number(speed)
        + QString::fromUtf8(" Байт/с"));
    lastSent = sent;
    measure.restart();
}
//------------------------------------------------------------------------------
void MainWindow::onCgLog(QString l)
{
    Q_UNUSED(l)
    qDebug() << l;
}
//------------------------------------------------------------------------------
void MainWindow::on_com_button_clicked()
{
    if(cg.isConnected())
    {
        cg.disconnectFromDevice();
        ui->com_button->setText(QString::fromUtf8("Подключить"));
        ui->com_combo->setEnabled(true);
    }
    else
    {
        int res = cg.connectToDevice(ui->com_combo->currentText().toLatin1().data(),
                           115200);
        if(res != CG_OK)
        {
            QMessageBox::critical(
                this,
                QString::fromUtf8("Ошибка подключения к устройству"),
                QString::fromUtf8(cg.str_error()));
            return;
        }
        ui->com_button->setText(QString::fromUtf8("Отключить"));
        ui->com_combo->setEnabled(false);
    }
}
//------------------------------------------------------------------------------
void MainWindow::on_net_button_clicked()
{
    if(srv.isListening())
    {
        closeServer();
        ui->net_button->setText(QString::fromUtf8("Открыть"));
        ui->log_edit->append(QString::fromUtf8("Сервер закрыт"));
        ui->port1_sb->setEnabled(true);
        ui->port2_sb->setEnabled(true);
    }
    else
    {
        if(srv.listen(QHostAddress::Any, ui->port1_sb->value()))
        {
            ui->log_edit->append(QString::fromUtf8("Сервер 1 открыт на порту ")
                             + QString::number(ui->port1_sb->value()));
        }
        if(ptsrv.listen(QHostAddress::Any, ui->port2_sb->value()))
        {
            ui->log_edit->append(QString::fromUtf8("Сервер 2 открыт на порту ")
                             + QString::number(ui->port2_sb->value()));
        }
        ui->net_button->setText(QString::fromUtf8("Закрыть"));
        ui->port1_sb->setEnabled(false);
        ui->port2_sb->setEnabled(false);
    }
}
//------------------------------------------------------------------------------
void MainWindow::closeServer()
{
    for(int a=0; a<clients.size(); a++)
    {
        clients[a]->deleteLater();
    }
    clients.clear();
    srv.close();

    for(int a=0; a<ptclients.size(); a++)
    {
        ptclients[a]->deleteLater();
    }
    ptclients.clear();
    ptsrv.close();
}
//------------------------------------------------------------------------------
void MainWindow::onNewConnection()
{
    QTcpSocket* s = srv.nextPendingConnection();
    if(s == 0)
        return;
    ui->log_edit->append(QString::fromUtf8("Сервер 1: Подключен клиент: ")
                         + s->peerAddress().toString() + ":"
                         + QString::number(s->peerPort()));

    connect(s, SIGNAL(disconnected()), SLOT(onClientDisconnected()));
    connect(s, SIGNAL(readyRead()), SLOT(onClientDataReady()));

    clients.append(s);
}
//------------------------------------------------------------------------------
void MainWindow::onNewPtConnection()
{
    QTcpSocket* s = ptsrv.nextPendingConnection();
    if(s == 0)
        return;
    ui->log_edit->append(QString::fromUtf8("Сервер 2: Подключен клиент: ")
                         + s->peerAddress().toString() + ":"
                         + QString::number(s->peerPort()));

    connect(s, SIGNAL(disconnected()), SLOT(onClientDisconnected()));
    connect(s, SIGNAL(readyRead()), SLOT(onPtDataReady()));

    ptclients.append(s);
}
//------------------------------------------------------------------------------
void MainWindow::onClientDisconnected()
{
    QObject *s = sender();
    if(s->inherits("QTcpSocket"))
    {
        QTcpSocket *ss = (QTcpSocket*)s;
        ss->disconnectFromHost();

        clients.removeAll(ss);
        ptclients.removeAll(ss);

        ss->deleteLater();
    }
    ui->log_edit->append(QString::fromUtf8("Клиент отключился"));
}
//------------------------------------------------------------------------------
void MainWindow::onClientDataReady()
{
    QObject *s = sender();
    if(s == 0)
        return;
    if(!s->inherits("QTcpSocket"))
        return;

    QTcpSocket *ss = (QTcpSocket*)s;
    ss->readAll();

    // Просто игнорируем все, что пришло
}
//------------------------------------------------------------------------------
void MainWindow::onPtDataReady()
{
    QObject *s = sender();
    if(s == 0)
        return;
    if(!s->inherits("QTcpSocket"))
        return;

    QTcpSocket *ss = (QTcpSocket*)s;
    ss->readAll();

    // Просто игнорируем все, что пришло
}
//------------------------------------------------------------------------------
void MainWindow::on_about_action_triggered()
{
    about.show();
}
//------------------------------------------------------------------------------
void MainWindow::on_refreshPorts_button_clicked()
{
    ui->com_combo->clear();
    #ifdef WIN32
    for(int a=0; a<100; a++)
    {
        char portname[8];
        sprintf(portname, "COM%d",a+1);
        char devname[1024];
        int err = QueryDosDeviceA(portname, devname, 1024);
        if(err == 0)
        {
            int e = GetLastError();
            if(e == ERROR_FILE_NOT_FOUND)
                continue;
            LPVOID lpMsgBuf;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                e,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );
            QMessageBox::warning(this, QString("Error"), QString("Unable to QueryDosDevice: (")
                                                         + QString::number(e) + ") "
                                                         + QString::fromWCharArray((wchar_t*)lpMsgBuf) );
            LocalFree(lpMsgBuf);
        }
        else
        {
            ui->com_combo->addItem(QString("\\\\.\\COM")+QString::number(a+1));
            int i = 0;
            QString s;
            while(i<err)
            {
                char c = devname[i];
                if(c == 0)
                {
                    qDebug() << s;
//                    ui->com_combo->addItem(s);
                    s = "";
                    i++;
                    if(devname[i] == 0)
                        break;
                    continue;
                }
                s.append(c);
                i++;
            }
        }
    }
    #else
    QDir d("/dev/");
    QStringList nf;
    nf << "ttyS*" << "ttyUSB*" << "ttyACM*";
    QStringList files = d.entryList(
        nf,
        QDir::Files | QDir::NoDotAndDotDot | QDir::System,
        QDir::Name);
    for(int a=0; a<files.size(); a++)
    {
        ui->com_combo->addItem(QString("/dev/") + files[a]);
    }
    #endif
}
//------------------------------------------------------------------------------
void MainWindow::on_resetRotation_button_clicked()
{
    cg.resetRotation();
}
//------------------------------------------------------------------------------
void MainWindow::on_calibrateZero_action_triggered()
{
    QMessageBox::information(
        this,
        QString::fromUtf8("Калибровка гироскопа"),
        QString::fromUtf8("Держите устройство неподвижно в течение нескольких секунд.")
        );
    cg.startDriftMeasure();
    calibTime.start();
    bZeroCalibration = true;
}
//------------------------------------------------------------------------------
void MainWindow::on_calibrateAngle_action_triggered()
{
    QMessageBox::information(
        this,
        QString::fromUtf8("Калибровка гироскопа"),
        QString::fromUtf8("Поверните устройство на 90 градусов"));
    bCoeffCalibration = true;
    calibTime.restart();
}
