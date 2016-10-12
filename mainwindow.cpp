#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtimer.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // add a graph to the plot and set it's color to blue:
    ui->customPlot1->addGraph();
    ui->customPlot1->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)1);

    ui->customPlot2->addGraph();
    ui->customPlot2->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlot2->graph(0)->setLineStyle((QCPGraph::LineStyle)1);

    // set x axis to be a time ticker and y axis to be from -1.5 to 1.5:
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot1->xAxis->setTicker(timeTicker);
    ui->customPlot1->axisRect()->setupFullAxesBox();
    ui->customPlot1->yAxis->setRange(-1.5, 1.5);

    ui->customPlot2->xAxis->setTicker(timeTicker);
    ui->customPlot2->axisRect()->setupFullAxesBox();
    ui->customPlot2->yAxis->setRange(-1.5, 1.5);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot1->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot1->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot1->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot1->yAxis2, SLOT(setRange(QCPRange)));

    connect(ui->customPlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot when the timer times out:
    QTimer *dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer->start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // set key to the time that has elasped from the start in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.005) // at most add point every 20 ms
    {
      // add data to lines:
      ui->customPlot1->graph(0)->addData(key, qSin(key));
      ui->customPlot2->graph(0)->addData(key, qSin(key));
      // TODO: add event for changing graph color based on value of graph.

      // rescale value (vertical) axis to fit the current data:
      ui->customPlot1->graph(0)->rescaleValueAxis();
      ui->customPlot2->graph(0)->rescaleValueAxis();
      lastPointKey = key;
    }
    // make key axis range scroll with the data:
    ui->customPlot1->xAxis->setRange(key, 10, Qt::AlignRight);
    ui->customPlot1->replot();

    ui->customPlot2->xAxis->setRange(key, 10, Qt::AlignRight);
    ui->customPlot2->replot();

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
}

MainWindow::~MainWindow()
{
    delete ui;
}
