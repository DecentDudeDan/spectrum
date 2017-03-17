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
    QVector<double> createDataPoints(bool isLinear);
    void setXAxis();
    void getPlotValues(QVector<double> points);
    void clearPoints();
    void resetValues();
    void setupGraph();
    void startStuff();
    void stopStuff();
    void refreshPlotting();
    void startPlotting();

private Q_SLOTS:
    void realtimeDataSlot();
    void on_startButton_clicked();
    void on_FFT1_currentIndexChanged(int index);
    void on_StopButton_clicked();
    void on_CF1_editingFinished();
    void on_spanValue_editingFinished();


private:
    Ui::MainWindow *ui;
    double CF;
    double AB;
    double S;
    double tempCF;
    int numPoints;
    int tempNumPoints;
    bool inSetup;
    QVector<double> xValue;
    QVector<double> plotPoints;
    QTimer *dataTimer;
    libThread* newThread;
};

#endif // MAINWINDOW_H
