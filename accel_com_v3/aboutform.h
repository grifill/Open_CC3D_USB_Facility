#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <QWidget>
#include <QPixmap>
#include <QShortcut>

#define APP_VERSION QString("3.0 RC1")

namespace Ui {
class AboutForm;
}

class AboutForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit AboutForm(QWidget *parent = 0);
    ~AboutForm();

    void setPixmap(QPixmap p);
    void setAppName(QString n);
    void setAppVersion(QString v);
    
private:
    Ui::AboutForm *ui;
    QShortcut* esc_shcut;
};

#endif // ABOUTFORM_H
