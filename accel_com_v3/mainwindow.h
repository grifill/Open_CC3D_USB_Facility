#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <stdint-gcc.h>
#include "comgate.h"
#include "aboutform.h"

/* Сервер 1 шлет всем клиентам 3 double-а с ускорениями (x, y, z). В ответ ничего
 * не берет.
 * Сервер 2 шлет всем клиентам координаты ползающих точек в формате:
 * int ID
 * int x
 * int y
 * После окончания точек высылается int z = -500
 * Принимаются следующие команды:
 * 0xFF 0x01 ID - добавление квадрата с заданным ID.
 * 0xFF 0x02 DIR - изменение направления движения квадрата, DIR - направление:
 *                 0 - стоп, 1 - вправо, 2 - вверх, 3 - влево, 4 - вниз.
*/

struct PTClient
{
    QTcpSocket* sock;
    int ID;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    AboutForm about;
    ComGate cg;
    Rotation rot;
    QTimer pokeTimer;
    QTimer speedTimer;
    QTcpServer srv;                 // Сервер варианта 1 (ускорения)
    QList<QTcpSocket*> clients;
    QTcpServer ptsrv;               // Сервер варианта 2 (координаты)
    QList<PTClient> ptclients;
    QTcpServer gyroSrv;             // Сервер варианта 3 (гироскоп)
    QList<QTcpSocket*> gyroClients;

    // Измерение скорости
    int64_t sent;
    int64_t lastSent;
    QTime measure;

    // Калибровка гироскопа
    QTime calibTime;
    bool bZeroCalibration;
    Rotation gyroDrift;
    bool bCoeffCalibration;

    // Измерение времени при работе без устройства
    QTime tick;
    int dataTime;

    // Закрывает сервер и отключает всех клиентов
    void closeServer();

private slots:
    void onPokeTimer();
    void onSpeedTimer();
    void onCgLog(QString l);
    void onNewConnection();
    void onClientDisconnected();    // Отключение общее для всех серверов
    void onClientDataReady();       // Dummy-версия, просто читает все что есть
    void onNewPtConnection();
    void onPtDataReady();
    void onNewGyroConnection();

    void on_com_button_clicked();
    void on_net_button_clicked();
    void on_about_action_triggered();
    void on_refreshPorts_button_clicked();
    void on_resetRotation_button_clicked();
    void on_calibrateZero_action_triggered();
    void on_calibrateAngle_action_triggered();
};

#endif // MAINWINDOW_H
