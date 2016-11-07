#include "glwidget.h"

#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include <QtDebug>

#include <QTime>

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    cx = -5;
    cy = -5;
    cz = 10;
    cp = 30;
    cr = -60;

    connect(&updateTimer, SIGNAL(timeout()), SLOT(updateGL()));
    updateTimer.start(20);
}

void GLWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);

    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Освещение
    GLfloat amb[] = {0.2, 0.2, 0.2, 1};
    GLfloat diff[] = {0.6, 0.6, 0.6, 1};
    GLfloat spec[] = {0, 0, 0, 0};
    GLfloat pos[] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
//    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01);
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL()
{
//    qDebug() << "Camera at" <<cx<<":"<<cy<<":"<<cz
//             <<"rotated"<<cp<<":"<<cr;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double aspect = (double)width() / (double)height();
//    gluPerspective(60, aspect, 1, 500);
    glFrustum(-aspect, aspect, -1, 1, 1.5, 500);

    glTranslatef(0, 0, -10);

    glRotatef(cr, 1, 0, 0);
    glRotatef(cp, 0, 0, 1);

//    glTranslatef(-cx, -cy, -cz);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);

    glLineWidth(2);
    // Оси координат
//    glColor3f(1, 0, 0);
    glColor3b(127, 0, 0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(10, 0, 0);
    glEnd();

    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 10, 0);
    glEnd();

    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 10);
    glEnd();

    glLineWidth(1);

//    // Серая подложка
//    glColor3f(0.5, 0.5, 0.5);
//    glBegin(GL_QUADS);
//        glVertex2f(-10, -10);
//        glVertex2f(-10, 10);
//        glVertex2f(10, 10);
//        glVertex2f(10, -10);
//    glEnd();

    glColor3f(1, 1, 1);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(ax, ay, az);
    glEnd();

    glEnable(GL_LIGHTING);

    // Плата
    glRotatef(pp, 0, 1, 0);
    glRotatef(pr, -1, 0, 0);
    glScalef(3, 2, 0.1);
    glColor3f(0.1, 0.7, 0.1);
    drawCube();

}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    lastpos = e->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint diff = e->pos() - lastpos;
    lastpos = e->pos();

    cp += 0.4*diff.x();
    cr += 0.4*diff.y();
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Up:
            cx += sin(cp/180.0*M_PI);
            cy += cos(cp/180.0*M_PI);
            break;
        case Qt::Key_Down:
            cx -= sin(cp/180.0*M_PI);
            cy -= cos(cp/180.0*M_PI);
            break;

        case Qt::Key_Left:
            cx -= cos(cp/180.0*M_PI);
            cy += sin(cp/180.0*M_PI);
            break;
        case Qt::Key_Right:
            cx += cos(cp/180.0*M_PI);
            cy -= sin(cp/180.0*M_PI);
            break;

        case Qt::Key_PageUp:
            cz += 1;
            break;
        case Qt::Key_PageDown:
            cz -= 1;
            break;

        default:
            break;
    }
}

void GLWidget::drawCube()
{
    GLfloat n[6][3] = {{0, 0, -1}, {0, 1, 0}, {0, 0, 1}, {0, -1, 0},
                       {1, 0, 0}, {-1, 0, 0}};
    int f[6][4] = {{0, 1, 2, 3}, {0, 1, 5, 4}, {4, 5, 6, 7},
                   {3, 7, 6, 2}, {0, 4, 7, 3}, {2, 6, 5, 1}};
    GLfloat v[8][3];

    v[0][0] = v[3][0] = v[4][0] = v[7][0] = 1;
    v[1][0] = v[2][0] = v[5][0] = v[6][0] = -1;

    v[0][1] = v[1][1] = v[4][1] = v[5][1] = 1;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = -1;

    v[0][2] = v[1][2] = v[2][2] = v[3][2] = -1;
    v[4][2] = v[5][2] = v[6][2] = v[7][2] = 1;

    glBegin(GL_QUADS);
    for(int a=0; a<6; a++)  // Цикл по граням
    {
        glNormal3fv(n[a]);
        for(int b=0; b<4; b++)  // Цикл по вершинам грани
        {
            glVertex3fv(v[f[a][b]]);
        }
    }
    glEnd();
}

void GLWidget::setPlateRotations(double pitch, double roll)
{
    pp = pitch;
    pr = roll;
}

void GLWidget::setAccels(double x, double y, double z)
{
    ax = x;
    ay = y;
    az = z;
}
