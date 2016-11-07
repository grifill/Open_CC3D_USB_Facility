#-------------------------------------------------
#
# Project created by QtCreator 2014-12-05T12:51:25
#
#-------------------------------------------------

QT       += core gui opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = accel_com
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        glwidget.cpp \
        pointwidget.cpp \
        comgate.cpp \
    aboutform.cpp

HEADERS  += mainwindow.h \
            comgate.h \
            glwidget.h \
            pointwidget.h \
    aboutform.h

FORMS    += mainwindow.ui \
    aboutform.ui

RESOURCES +=
