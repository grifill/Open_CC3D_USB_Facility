#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QTimer>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void setPlateRotations(double pitch = 0, double roll = 0, double yaw = 0);

    void setRotationMatrix(GLdouble* m);

    void setAccels(double x, double y, double z);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent* e);

signals:
    
public slots:
    
private:
    QPoint lastpos;
    QTimer updateTimer;
    // Поворот камеры
    double cp, cr;
    // Координаты камеры
    double cx, cy, cz;

    // Углы поворота платы
    double pp, pr, py;

    // Матрица поворота платы
    GLdouble rotMatrix[16];
    bool bMatrixRotation;

    // Вектор ускорения
    double ax, ay, az;

    void drawCube();
    void drawPlate();
};

#endif // GLWIDGET_H
