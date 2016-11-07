#include "aboutform.h"
#include "ui_aboutform.h"

AboutForm::AboutForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutForm)
{
    ui->setupUi(this);
    hide();
    ui->icon_label->hide();
    ui->author_label->setSelection(0,0);

    esc_shcut=new QShortcut(Qt::Key_Escape, this);
    connect(esc_shcut, SIGNAL(activated()), SLOT(close()));

    int qt_maj = (QT_VERSION & 0xFF0000) >> 16;
    int qt_min = (QT_VERSION & 0xFF00) >> 8;
    int qt_patch = QT_VERSION & 0xFF;
    ui->qtversion_label->setText(QString::fromUtf8("Собрано на Qt ")
                                 + QString::number(qt_maj) + "."
                                 + QString::number(qt_min) + "."
                                 + QString::number(qt_patch) );

    setAppVersion(APP_VERSION);
}
//------------------------------------------------------------------------------
AboutForm::~AboutForm()
{
    delete ui;
    delete esc_shcut;
}
//------------------------------------------------------------------------------
void AboutForm::setPixmap(QPixmap p)
{
    ui->icon_label->setPixmap(p);
}
//------------------------------------------------------------------------------
void AboutForm::setAppName(QString n)
{
    ui->appName_label->setText(n);
    setWindowTitle(n);
}
//------------------------------------------------------------------------------
void AboutForm::setAppVersion(QString v)
{
    ui->appVersion_label->setText(QString::fromUtf8("Версия ") + v);
}
