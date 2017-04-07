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
    void getPlotValues(QVector<QVector<double>> points);
    void clearPoints();
    void updateInfo();
    void resetValues();
    void setupGraph();
    void setGUIValues();
    void startStuff();
    void stopStuff();
    void refreshPlotting();
    void startPlotting();
    void endRunningThread();
    void setupWindowingVectors();

private Q_SLOTS:
    void realtimeDataSlot();
    void on_startButton_clicked();
    void on_FFT1_currentIndexChanged(int index);
    void on_StopButton_clicked();
    void on_CF1_editingFinished();
    void on_AB1_editingFinished();
    void on_Span1_editingFinished();
    void on_AVG1_editingFinished();
    void on_CF2_currentTextChanged(const QString &arg1);
    void on_Span2_currentTextChanged(const QString &arg1);
    void on_WSize_currentIndexChanged(int index);
    void on_Theme1_currentIndexChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    double CF;
    double AB;
    double S;
    double tempCF;
    double tempAB;
    int numPoints;
    int tempNumPoints;
    int numberOfAverages;
    double maxPoint;
    double cfMhz;
    double spanMhz;
    bool inSetup;
    int windowType;
    QVector<double> xValue;
    QVector<double> plotPoints;
    QVector<double> windowMult;
    QTimer *dataTimer;
    libThread* newThread;
};

#endif // MAINWINDOW_H
