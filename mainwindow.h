#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>
#include <QDir>
#include "libthread.h"
#include "qtimer.h"
#include <fftw3.h>
#include "speccursor.h"

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
    void ManageCursor(QCustomPlot* plot, double x, double y, QPen pen, bool firstLine);
    int getIndexFromHertz(double hertz);


private Q_SLOTS:
    void realtimeDataSlot();
    void on_startButton_clicked();
    void on_FFT1_currentIndexChanged(int index);
    void on_StopButton_clicked();
    void on_CF1_editingFinished();
    void on_AB1_editingFinished();
    void on_Span1_editingFinished();
    void on_CF2_currentTextChanged(const QString &arg1);
    void on_Span2_currentTextChanged(const QString &arg1);
    void on_WSize_currentIndexChanged(int index);
    void on_Theme1_currentIndexChanged(const QString &arg1);
    void on_Grid1_currentIndexChanged(const QString &arg1);
    void on_Mode1_currentIndexChanged(const QString &arg1);
    void on_Settings_clicked();
    void on_w3close_clicked();
    void on_Export_clicked();
    void on_AVG1_currentTextChanged(const QString &arg1);
    void mousePress(QMouseEvent* event);
    void mouseMove(QMouseEvent* event);
    void mouseRelease(QMouseEvent* event);


private:
    Ui::MainWindow *ui;
    double CF;
    double AB;
    double S;
    int numPoints;
    int tempNumPoints;
    int numberOfAverages;
    double maxFrequency;
    double maxPoint;
    double cfMhz;
    double spanMhz;
    bool firstRun;
    bool isLinear;
    bool mouseHeld;
    int windowType;
    double v1Index;
    double v2Index;
    QVector<double> xValue;
    QVector<double> plotPoints;
    QVector<double> windowMult;
    QVector<std::complex<double>> cleanPoints;
    QTimer *dataTimer;
    libThread* newThread;
    SpecCursor* cursor;
};

#endif // MAINWINDOW_H
