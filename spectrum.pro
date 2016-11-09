#-------------------------------------------------
#
# Project created by QtCreator 2016-09-21T10:52:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = spectrum
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h
FORMS    += mainwindow.ui

LIBS += -L/home/dan/Documents/seniorproject/fftw-3.3.5 -L/home/dan/Documents/seniorproject/libiio \
-lfftw3 \
-liio
