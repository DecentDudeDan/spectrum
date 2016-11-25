#-------------------------------------------------
#
# Project created by QtCreator 2016-09-21T10:52:41
#
#-------------------------------------------------

QT       += core gui
QT += widgets concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = spectrum
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    concurrentqueue.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    concurrentqueue.h
FORMS    += mainwindow.ui

LIBS += -L/home/chris/fftw-3.3.5/fftw-3.3.5 -L/home/chris/libiio-master\
-lfftw3 \
-liio \
