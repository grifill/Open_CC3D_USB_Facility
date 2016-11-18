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
    cz = 20;
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
    GLfloat diff[] = {0.4, 0.4, 0.4, 1};
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

    glTranslatef(0, 0, -cz);

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

    // Вектор ускорения
    float al = 0.08*sqrt(ax*ax + ay*ay + az*az);
    glColor3f(1, 1, 1);
    glLineWidth(3);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(ax/al, ay/al, az/al);
    glEnd();
    glLineWidth(0);

    glEnable(GL_LIGHTING);

    // Поворот платы
    glRotatef(py, 0, 0, 1);
    glRotatef(pp, 0, 1, 0);
    glRotatef(pr, -1, 0, 0);
    // Плата
    drawPlate();
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

void GLWidget::wheelEvent(QWheelEvent *e)
{
    float step = e->delta();
    cz -= 0.01*step;
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

void GLWidget::setPlateRotations(double pitch, double roll, double yaw)
{
    pp = pitch;
    pr = roll;
    py = yaw;
}

void GLWidget::setAccels(double x, double y, double z)
{
    ax = x;
    ay = y;
    az = z;
}

void GLWidget::drawPlate()
{
    // Сама плата
    glPushMatrix();
    glScalef(5, 5, 0.2);
    glColor3f(0.8, 0.8, 0.8);
    drawCube();
    glPopMatrix();

    // Разъем USB
    glPushMatrix();
    glTranslated(0, -4, -0.5);
    glScaled(1, 1, 0.3);
    glColor3f(0.9, 0.9, 0.9);
    drawCube();
    glPopMatrix();

    // Процессор
    glPushMatrix();
    glTranslated(0.1, -0.5, 0.3);
    glScaled(1, 1, 0.1);
    glColor3f(0.1, 0.1, 0.1);
    drawCube();
    glPopMatrix();

    // Микруха ближе к разъему
    glPushMatrix();
    glTranslated(0, -3, 0.3);
    glScaled(0.6, 0.75, 0.1);
    glColor3f(0.1, 0.1, 0.1);
    drawCube();
    glPopMatrix();

    // Микруха дальше от разъема
    glPushMatrix();
    glTranslated(0.3, 3.5, 0.3);
    glScaled(0.6, 0.6, 0.1);
    glColor3f(0.1, 0.1, 0.1);
    drawCube();
    glPopMatrix();

    const float gridStep_x = 0.31;    // Шаг рейки разъемов
    const float gridStep_y = 0.35;    // Шаг рейки разъемов
    const float gridX = 3.5;        // Центр сетки
    const float gridY = 1;

    // Подложка контактной рейки
    glPushMatrix();
    glTranslated(gridX, gridY, 0.3);
    glScaled(3*gridStep_x, 6*gridStep_y, 0.15);
    glColor3f(0.1, 0.1, 0.1);
    drawCube();
    glPopMatrix();

    glLineWidth(5);
    glColor3f(0.9, 0.9, 0.2);
    // Контактная рейка
    for(int x=0 ; x<3; x++)
    {
        for(int y=0; y<6; y++)
        {
            glPushMatrix();
            glTranslated(gridX + 2*gridStep_x*((float)x - 1),
                         gridY + 2*gridStep_y*((float)y - 2.5),
                         1
                        );
            glScaled(0.07, 0.07, 1);
            drawCube();
            glPopMatrix();
        }
    }
}
