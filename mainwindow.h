#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>
#include <QtConcurrent/QtConcurrent>
#include "libthread.h"
#include "qtimer.h"
#include <fftw3.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QVector<double> createDataPoints();
    void clearPoints();
    void resetXValues();
    void setupGraph();
    void startStuff();
    void stopStuff();

private Q_SLOTS:
    void realtimeDataSlot();
    void on_startButton_clicked();
    void on_FFT1_currentIndexChanged(int index);
    void on_StopButton_clicked();
    void on_CF1_editingFinished();
    void on_AB1_editingFinished();

private:
    Ui::MainWindow *ui;
    double CF;
    double AB;
    int numPoints;
    bool inSetup;
    QVector<double> xValue;
    QTimer *dataTimer;
    libThread* newThread;
};

#endif // MAINWINDOW_H
