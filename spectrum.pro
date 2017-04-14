#-------------------------------------------------
#
# Project created by QtCreator 2016-09-21T10:52:41
#
#-------------------------------------------------

QT       += core gui
QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = spectrum
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    qcustomplot.cpp \
    concurrentqueue.cpp \
    libthread.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    concurrentqueue.h \
    libthread.h

FORMS    += mainwindow.ui

LIBS += -L/home/timmy/FFTW/fftw-3.3.5/fftw-3.3.5 -L/home/timmy/libiio/libiio \
-lfftw3 \
-liio \
