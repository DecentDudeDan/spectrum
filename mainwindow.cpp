#include "mainwindow.h"
#include "ui_mainwindow.h"

#define initialS 60;
double cfMhz = 0;
double spanMhz= 0;

ConcurrentQueue* points = new ConcurrentQueue();
bool isShuttingDown;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    CF(2.5),
    AB(60),
    S(60),
    numPoints(512),
    tempNumPoints(512),
    numberOfAverages(1),
    maxPoint(0),
    inSetup(true)
{
    dataTimer = new QTimer();
    ui->setupUi(this);

    newThread = new libThread(numPoints, AB, CF);

    setupGraph();

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot when the timer times out:
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(ui->StopButton, SIGNAL(clicked()), dataTimer, SLOT(stop()));
    //setup user inputs dropdown values
    ui->FFT1->addItem("256", QVariant(256));
    ui->FFT1->addItem("512", QVariant(512));
    ui->FFT1->addItem("1024", QVariant(1024));
    ui->FFT1->addItem("2048", QVariant(2048));
    ui->FFT1->addItem("4096", QVariant(4096));
    ui->FFT1->addItem("8192", QVariant(8192));
    ui->FFT1->addItem("16384", QVariant(16384));
    ui->FFT1->addItem("32768", QVariant(32768));
    ui->FFT1->addItem("65536", QVariant(65536));
    ui->WSize->addItem("Rectangular");
    ui->WSize->addItem("Blackman's");
    ui->WSize->addItem("Blacktop");
    ui->WSize->addItem("Hanning");
    ui->WSize->addItem("Hamming");
    
    inSetup = false;

    ui->CF2->addItem("GHz");
    ui->CF2->addItem("MHz");

    ui->Span2->addItem("MHz");
    ui->Span2->addItem("kHz");

    ui->Mode1->addItem("Logrithmatic");
    ui->Mode1->addItem("Linear");

    ui->Grid1->addItem("On");
    ui->Grid1->addItem("Off");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupGraph()
{

    ui->customPlot1->setBackground(Qt::lightGray);
    ui->customPlot1->axisRect()->setBackground(Qt::black);

    ui->FFT->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");
    ui->CF->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");
    ui->AB->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");

    // add a graph to the plot and set it's color to blue:
    ui->customPlot1->addGraph();
    ui->customPlot1->graph(0)->setPen(QPen(QColor(224, 195, 30)));
    ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)2);

    // set x axis to be a time ticker and y axis to be from -1.5 to 1.5:
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    setXAxis();
    ui->customPlot1->axisRect()->setupFullAxesBox();
    ui->customPlot1->yAxis->setRange(-350, 0);
}

void MainWindow::setXAxis()
{
    double start = CF - S/2;
    double end = CF + S/2;

    ui->customPlot1->xAxis->setRange(start, end);
}

void MainWindow::stopStuff()
{
    dataTimer->stop();
    newThread->setStop(true);
    newThread->exit();
    clearPoints();
    ui->customPlot1->clearGraphs();
}

void MainWindow::startStuff()
{
    if (newThread->isRunning())
    {
        stopStuff();
        qDebug() << "is finished: " << newThread->isFinished();
        if (newThread->isFinished()) {
            refreshPlotting();
        }
    } else {
        refreshPlotting();
    }
}

void MainWindow::refreshPlotting()
{
    updateInfo();
    setupGraph();
    resetValues();
    startPlotting();
}

void MainWindow::startPlotting()
{
    newThread = new libThread(numPoints, AB, CF);
    newThread->start();
    dataTimer->start();
}

void MainWindow::updateInfo()
{
    numPoints = tempNumPoints;
}

void MainWindow::clearPoints()
{
    delete points;
    points = new ConcurrentQueue();
}

void MainWindow::resetValues()
{
    if(xValue.size() > 0) {
        xValue.clear();
    }

    if(plotPoints.size() > 0) {
        plotPoints.clear();
    }

}

void MainWindow::endRunningThread()
{
    if(newThread->isRunning())
    {
        stopStuff();
    }
}

void MainWindow::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    QVector<double> tmpfftPoints;
    double key;
    static double lastPointKey;
    QVector<QVector<double>> fftPoints;

    for (int i = 0; i < numberOfAverages; i++)
    {
        if(points->size() > numPoints)  {
            tmpfftPoints = createDataPoints(false);
            fftPoints.push_back(tmpfftPoints);
        }
    }

    if(fftPoints.size() > 0)
    {
        getPlotValues(fftPoints);
    }

    key = time.elapsed()/1000.0; // set key to the time that has elasped from the start in seconds

    if (key-lastPointKey > 0.006)
    {

        // add data to lines:
        if (xValue.size() == plotPoints.size() && plotPoints.size() > 0) {
            ui->customPlot1->graph(0)->setData(xValue, plotPoints);
        }

        // rescale value (vertical) axis to fit the current data:
        //ui->customPlot1->graph(0)->rescaleValueAxis();

        ui->customPlot1->replot();
        lastPointKey = key;
    }

    // calculate frames per second and add it to the ui window at bottom of the screen:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2, number of vectors: %3, plotPoints: %4, xValues: %5, max Point Overall: %5, size of vector")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot1->graph(0)->data()->size())
                    .arg(fftPoints.size())
                    .arg(plotPoints.size())
                    .arg(xValue.size())
                    .arg(maxPoint)
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }

}

void MainWindow::getPlotValues(QVector<QVector<double>> points)
{
    xValue.clear();
    plotPoints.clear();
    double xHertz;
    double dPoints = numPoints;
    double temp = ((60-S)/2);
    double startIndex = (dPoints/60)*temp;
    double temp2 = ((60+S)/2);
    double endIndex = (dPoints/60)*temp2;
    double xinc = 0;

    double shift = S/2;

    if (endIndex <= points[0].size())
    {
        for(int i = startIndex; i < endIndex; i++ )
        {
            xinc = i - startIndex;
            xHertz = ((CF-shift) + (xinc * (60/dPoints)));
            xValue.push_back(xHertz);
            double avgPoint = 0;
            if (points.size() > 1)
            {
                for (int j = 0; j < points.size(); j++)
                {
                    avgPoint += points[j].at(i);
                    if(points[j].at(i) > maxPoint)
                    {
                        maxPoint = points[j].at(i);
                    }
                }
                avgPoint = avgPoint/points.size();
                plotPoints.push_back(avgPoint);
            } else
            {
                plotPoints.push_back(points[0].at(i));
            }


        }
    }

}

QVector<double> MainWindow::createDataPoints(bool isLinear)
{
    int i;
    QVector<double> fftPoints;
    QVector<double> ffttemp1;
    double dPoints = numPoints;
    fftw_complex in[numPoints], out[numPoints];
    fftw_plan p;

    p = fftw_plan_dft_1d(numPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    if(points->size() > numPoints) {
        for (i = 0; i < numPoints; i++)
        {
            std::complex<double> current = points->dequeue();
            in[i][0] = current.real()/dPoints;
            in[i][1] = current.imag()/dPoints;
        }
    }

    fftw_execute(p);

    for (i = 0; i < dPoints; i++)
    {
        if (i < dPoints/2)
        {
            double Ppp = (out[i][0]*out[i][0] + out[i][1]*out[i][1])/(dPoints*dPoints);
            double dBFS = 10*log(Ppp);
            isLinear ? ffttemp1.push_back(Ppp) : ffttemp1.push_back(dBFS);
        } else
        {
            double Ppp = (out[i][0]*out[i][0] + out[i][1]*out[i][1])/(dPoints*dPoints);
            double dBFS = 10*log(Ppp);
            isLinear ? fftPoints.push_back(Ppp) : fftPoints.push_back(dBFS);
        }
    }

    //        for (i = 0; i < dPoints; i++)
    //        {
    //            if (i < dPoints/2)
    //            {
    //                double Ppp = sqrt((out[i][0]*out[i][0] + out[i][1]*out[i][1]));
    //                ffttemp1.push_back(Ppp);
    //            } else
    //            {
    //                double Ppp = sqrt((out[i][0]*out[i][0] + out[i][1]*out[i][1]));
    //                fftPoints.push_back(Ppp);
    //            }
    //        }

    fftPoints.append(ffttemp1);

    fftw_destroy_plan(p);

    return fftPoints;

}

void MainWindow::on_FFT1_currentIndexChanged(int index)
{
    endRunningThread();
    tempNumPoints = ui->FFT1->itemData(index).toInt();
    QThread::currentThread()->wait(1);
    if (tempNumPoints >= 256)
    {
        refreshPlotting();
    }
}

void MainWindow::on_Span1_editingFinished()
{
    double tSpan = ui->Span1->text().toDouble();

    if (spanMhz == 1)
    {
        if( tSpan <= 60)
        {
            S = tSpan;
            setupGraph();
            QMessageBox::about(this, "correct Value", "Correct value MHZ");
        }
         else
         {
            QMessageBox::about(this, "Incorrect Value", "Enter a value between 100 and 5970");
         }
    }
    if(spanMhz != 1)
    {
        if ( tSpan <= 60)
        {
            S = tSpan;
            setupGraph();
            QMessageBox::about(this, "Incorrect Value", "Correct value KHZ");
        }
        else
        {
            QMessageBox::about(this, "Incorrect Value", "Enter a number between .1 and 5.97");
        }
    }
}


void MainWindow::on_startButton_clicked()
{
    startStuff();
}

void MainWindow::on_StopButton_clicked()
{
    endRunningThread();
}

void MainWindow::on_CF1_editingFinished()
{
    double tCF = ui->CF1->text().toDouble();
    if (cfMhz == 1)
    {
        if( tCF >= 100 && tCF <= 5970)
        {
            endRunningThread();
            CF = tCF;
            refreshPlotting();
            QMessageBox::about(this, "correct Value", "Correct value MHZ");
        }

        else
        {
            QMessageBox::about(this, "Incorrect Value", "Enter a value between 100 and 5970");
        }
     }
     if(cfMhz != 1)
     {
        if ( tCF >= .1 && tCF <= 5.97)
        {
            endRunningThread();
            CF = tCF;
            refreshPlotting();
            QMessageBox::about(this, "Incorrect Value", "Correct value KHZ");
         }
        else
        {
            QMessageBox::about(this, "Incorrect Value", "Enter a number between .1 and 5.97");
        }
     }
 }


void MainWindow::on_AB1_editingFinished()
{
    double tAB = ui->AB1->text().toDouble();
    if (tAB)
    {
        tempAB = tAB;
    }
}

void MainWindow::on_AVG1_editingFinished()
{
    int tAvg = ui->AVG1->text().toInt();
    if(tAvg > 0 && tAvg < 10)
    {
        endRunningThread();
        numberOfAverages = tAvg;
        refreshPlotting();
    }
    else
    {
        QMessageBox::about(this, "Incorrect Value", "Enter a number 0 and 10");
    }
}





void MainWindow::on_CF2_currentTextChanged(const QString &arg1)
{
    if (ui->CF2->currentText() == "MHz")
        {
            cfMhz= 1;
        }
        else
        {
            cfMhz = 0;
        }
}

void MainWindow::on_Span2_currentTextChanged(const QString &arg1)
{

    if (ui->Span2->currentText() == "kHz")
       {
           spanMhz= 1;
       }
       else
       {
           spanMhz = 0;
       }
}
