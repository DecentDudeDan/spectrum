#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtimer.h"
#include <fftw3.h>
#include <iostream>
#include <math.h>
#include <QVector>
#define blah 256

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // add a graph to the plot and set it's color to blue:
    ui->customPlot1->addGraph();
    ui->customPlot1->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)2);

    ui->customPlot2->addGraph();
    ui->customPlot2->graph(0)->setPen(QPen(QColor(40, 255, 255)));
    ui->customPlot2->graph(0)->setLineStyle((QCPGraph::LineStyle)1);

    // set x axis to be a time ticker and y axis to be from -1.5 to 1.5:
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot1->xAxis->setRange(0,blah);
    ui->customPlot1->axisRect()->setupFullAxesBox();
    ui->customPlot1->yAxis->setRange(-1.5, 1.5);
    ui->customPlot1->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot1->yAxis->setTicker(logTicker);

    ui->customPlot2->xAxis->setTicker(timeTicker);
    ui->customPlot2->axisRect()->setupFullAxesBox();
    ui->customPlot2->yAxis->setRange(-1.5, 1.5);

    // make left and bottom axes transfer their ranges to right and top axes:

    connect(ui->customPlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot when the timer times out:
    QTimer *dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(ui->StopButton, SIGNAL(clicked()),dataTimer, SLOT(stop()));
    connect(ui->startButton, SIGNAL(clicked()),dataTimer, SLOT(start()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    double key;
    int Low = 1;
    int High = 10;
    int randNum;
    static double lastPointKey = 0;
    // calculate two new data points:

    randNum = (qrand() % ((High + 1) - Low) + Low);
    std::complex<double> *newPoints = new std::complex<double>[blah];

    for (int i = 0; i < blah; i++)
    {
        newPoints[i] = cos(1 * 2*M_PI*i/(blah));
    }

    QVector<double> fftPoints;
    QVector<double> xValue;
    createDataPoints(newPoints, xValue, fftPoints);

    key = time.elapsed()/1000.0; // set key to the time that has elasped from the start in seconds
    if (key-lastPointKey > 0.0002) // at most add point every 20 ms
    {
        // add data to lines:
        ui->customPlot1->graph(0)->setData(xValue, fftPoints);
        // TODO: add event for changing graph color based on value of graph.

        // rescale value (vertical) axis to fit the current data:
        ui->customPlot1->graph(0)->rescaleValueAxis();
        lastPointKey = key;
    }

    // make key axis range scroll with the data:
    //ui->customPlot1->xAxis->setRange(key, 5, Qt::AlignRight);
    ui->customPlot1->replot();
    // calculate frames per second and add it to the ui window at bottom of the screen:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot1->graph(0)->data()->size())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }

    delete newPoints;
}

void MainWindow::createDataPoints(std::complex<double> *points, QVector<double> &xValue, QVector<double> &fftPoints)
{
    int i;
    fftw_complex in[blah], out[blah];
    fftw_plan p;

    p = fftw_plan_dft_1d(blah, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (i = 0; i < blah; i++)
    {
        in[i][0] = points[i].real();
        in[i][1] = points[i].imag();
    }

    fftw_execute(p);

    for (i = 0; i < blah; i++)
    {
        fftPoints.push_back(sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]));
        xValue.push_back(i);
    }

    fftw_destroy_plan(p);

}


