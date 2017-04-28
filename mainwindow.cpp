#include "mainwindow.h"
#include "ui_mainwindow.h"

ConcurrentQueue* points = new ConcurrentQueue();

const double MILLION = 1000000.0;
const double THOUSAND = 1000.0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    CF(2.500),
    AB(60),
    S(.06),
    numPoints(8192),
    tempNumPoints(8192),
    numberOfAverages(1),
    maxFrequency(0),
    maxFrequency1(0),
    maxFrequency2(0),
    maxPoint(-200),
    cf2Mem(cfMhz),
    c1PowerVal(0),
    c2PowerVal(0),
    horzDelt(0),
    cfMhz(0),
    spanMhz(0),
    firstRun(true),
    isLinear(false),
    mouseHeld(false)
{
    dataTimer = new QTimer();
    cursor = new SpecCursor();

    ui->setupUi(this);

    newThread = new libThread(numPoints, AB, CF);

    setupGraph();
    setGUIValues();
    ui->widget->show();

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot when the timer times out:
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(ui->StopButton, SIGNAL(clicked()), dataTimer, SLOT(stop()));
    connect(ui->customPlot1, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
    connect(ui->customPlot1, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    connect(ui->customPlot1, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));

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
    ui->FFT1->setCurrentIndex(5);

    ui->WSize->addItem("Rectangular");
    ui->WSize->addItem("Blackman");
    ui->WSize->addItem("Flat Top");
    ui->WSize->addItem("Hanning");
    ui->WSize->addItem("Hamming");
    ui->Theme1->addItem("Dark");
    ui->Theme1->addItem("White");


    ui->AVG1->addItem("1",QVariant(1));
    ui->AVG1->addItem("2",QVariant(2));
    ui->AVG1->addItem("3",QVariant(3));
    ui->AVG1->addItem("4",QVariant(4));
    ui->AVG1->addItem("5",QVariant(5));
    ui->AVG1->addItem("6",QVariant(6));
    ui->AVG1->addItem("7",QVariant(7));
    ui->AVG1->addItem("8",QVariant(8));
    ui->AVG1->addItem("9",QVariant(9));
    ui->AVG1->addItem("10",QVariant(10));

    ui->Cursor1->addItem("Off");
    ui->Cursor1->addItem("On");


    firstRun = false;


    ui->CF2->addItem("GHz");
    ui->CF2->addItem("MHz");

    ui->Span2->addItem("MHz");
    ui->Span2->addItem("kHz");


    // ui->Mode1->addItem("Vrms");
    ui->Mode1->addItem("Logarithmic");
    ui->Mode1->addItem("Linear");
    //ui->Mode1->addItem("Watts");
    //ui->Mode1->addItem("dBm");

    ui->Grid1->addItem("On");
    ui->Grid1->addItem("Off");

    firstRun = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupGraph()
{

    ui->customPlot1->setBackground(Qt::lightGray);
    //ui->customPlot1->axisRect()->setBackground(Qt::black);

    ui->FFT->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");
    ui->CF->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");
    ui->AB->setStyleSheet("background-color: rgba( 255, 255, 255, 0);");

    //adds the graph
    ui->customPlot1->addGraph();
    // ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)2);
    //adds the line to the central frequency graph
    // QVector<double> YA(5000), XA(5000);

    //    for (int i = 0; i < 5000; i++)
    //    {
    //        YA[i] = i - 2500;
    //        XA[i] = ui->CF1->text().toDouble();
    //    }
    //    ui->customPlot1->addGraph();
    //    ui->customPlot1->graph(1)->setPen(QPen(Qt::white));
    //    ui->customPlot1->graph(1)->setData(XA, YA);
    //    if(ui->CF2->currentText() == "MHz")
    //    {
    //        for (int i = 0; i < 5000; i++)
    //        {
    //            YA[i] = i - 2500;
    //            XA[i] = (ui->CF1->text().toDouble())/1000;
    //        }
    //        ui->customPlot1->addGraph();
    //        ui->customPlot1->graph(1)->setPen(QPen(Qt::white));
    //        ui->customPlot1->graph(1)->setData(XA, YA);
    //    }

    ui->customPlot1->xAxis->setLabel("GHz");
    if (ui->CF2->currentText() == "MHz")
        ui->customPlot1->xAxis->setLabel("MHz");
    //if (ui->Mode1->currentText()=="V")
    //switch(ui->Mode1->currentText()){
    //    case V:      ui->customPlot1->yAxis->setLabel("V");
    //    case Vrms:   ui->customPlot1->yAxis->setLabel("Vrms");
    //    case dBV:    ui->customPlot1->yAxis->setLabel("dBV");
    //    case Watts:  ui->customPlot1->yAxis->setLabel("Watts");
    //    case dBm:    ui->customPlot1->yAxis->setLabel("dBm");
    //}


    //Makes sure the current theme set does not change
    if (ui->Theme1->currentText() == "Dark")
    {
        ui->customPlot1->axisRect()->setBackground(Qt::black);
        ui->customPlot1->graph(0)->setPen(QPen(QColor(224, 195, 30)));
    }
    else if (ui->Theme1->currentText() == "White")
    {
        ui->customPlot1->axisRect()->setBackground(Qt::white);
        ui->customPlot1->graph(0)->setPen(QPen(QColor(30, 119, 227)));
    }

    if (ui->Grid1->currentText() == "On")
    {
        ui->customPlot1->yAxis->setVisible(true);
        ui->customPlot1->yAxis->setTickLabels(true);
        ui->customPlot1->xAxis->setVisible(true);
        ui->customPlot1->xAxis->setTickLabels(true);
    }
    else if (ui->Grid1->currentText() == "Off")
    {
        ui->customPlot1->yAxis->setVisible(false);
        ui->customPlot1->xAxis->setVisible(false);
    }

    // set x axis to be a time ticker and y axis to be from -1.5 to 1.5:
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    setXAxis();
    ui->customPlot1->axisRect()->setupFullAxesBox();
    if (!isLinear)
    {
        ui->customPlot1->yAxis->setRange(-50, 50);
    } else
    {
        ui->customPlot1->yAxis->setRange(-0.001,0.15);
    }

    ui->HorzDeltaLabel->setText("MHz");
    setupWindowingVectors();
}

void MainWindow::ManageCursor(QCustomPlot* plot, double x, QPen pen, bool firstLine)
{

    if(firstLine)
    {
        if(cursor->getVLine1())
        {
            plot->removeItem(cursor->getVLine1());
        }
        cursor->setVLine1(new QCPItemLine(plot));
        cursor->getVLine1()->setPen(pen);
        cursor->getVLine1()->start->setCoords( x, QCPRange::maxRange);
        cursor->getVLine1()->end->setCoords( x, THOUSAND);
        v1Index = getIndexFromHertz(x);

        ui->MP1->setText(QString::number(x));
        c1PowerVal = x;


    } else
    {
        if(cursor->getVLine2())
        {
            plot->removeItem(cursor->getVLine2());
        }
        cursor->setVLine2(new QCPItemLine(plot));
        cursor->getVLine2()->setPen(pen);
        cursor->getVLine2()->start->setCoords( x, QCPRange::maxRange);
        cursor->getVLine2()->end->setCoords( x, THOUSAND);
        v2Index = getIndexFromHertz(x);

        ui->C2MP1->setText(QString::number(x));
        c2PowerVal = x;

    }
    if (cfMhz!=1)
    {
        horzDelt = fabs((c1PowerVal -c2PowerVal)*1000);
    }
    else
        horzDelt = fabs((c1PowerVal - c2PowerVal));

    ui->HorzDeltaBox->setText(QString::number(horzDelt));

}

float MainWindow::getOffset(float freq)
{
    float testvariable = 0;
    /*
     * x^7  3.618441430295573e-24
     * x^6  -4.946835169104284e-20
     * x^5  1.765187136354818e-16
     * x^4  1.954410451450048e-13
     * x^3  -1.981708419485009e-09
     * x^2  3.447607881262498e-06
     * x^1  -0.006032396479099
     * x^0  -8.525161305426837
     * */
    testvariable = 3.618e-24*pow(freq, 7.0);
    testvariable -= 4.946e-20*pow(freq, 6.0);
    testvariable += 1.765e-16*pow(freq, 5.0);
    testvariable += 1.954e-13*pow(freq, 4.0);
    testvariable -= 1.981e-09*pow(freq, 3.0);
    testvariable += 3.447e-06*pow(freq, 2.0);
    testvariable -= 6.032e-3*freq;
    testvariable -= 8.525;
    return testvariable;
}

void MainWindow::mousePress(QMouseEvent* event)
{
    if (ui->Cursor1->currentIndex() == 1)
    {
        mouseHeld = true;
        cursor->setCursorEnabled(true);

        QCustomPlot* plot = ui->customPlot1;
        double x=plot->xAxis->pixelToCoord(event->pos().x());
        if(event->buttons() == Qt::LeftButton)
        {
            ManageCursor(plot, x, QPen(Qt::green), true);
        }
        else
        {
            ManageCursor(plot, x, QPen(Qt::green), false);
        }
        plot->replot();
    }
}

void MainWindow::mouseMove(QMouseEvent* event)
{
    if (ui->Cursor1->currentIndex() == 1)
    {
        if (mouseHeld)
        {
            QCustomPlot* plot = ui->customPlot1;
            double x=plot->xAxis->pixelToCoord(event->pos().x());
            if(event->buttons() == Qt::LeftButton)
            {
                ManageCursor(plot, x, QPen(Qt::green), true);
            }
            else
            {
                ManageCursor(plot, x, QPen(Qt::green), false);
            }
            plot->replot();
        }
    }
}

void MainWindow::mouseRelease(QMouseEvent* event)
{
    mouseHeld = false;
}

void MainWindow::setGUIValues()
{
    ui->CF1->setText(QString::number(CF));
    ui->AB1->setText(QString::number(AB));

    if (spanMhz == 0)
    {
        ui->Span1->setText(QString::number(S*THOUSAND));
    } else
    {
        ui->Span1->setText(QString::number(S*MILLION));
    }
}

void MainWindow::setXAxis()
{

    double start = CF - S/2;
    double end = CF + S/2;
    if (cfMhz==1)
    {
        start=CF*1000-S*500;
        end = CF*1000+S*500;
    }
    ui->customPlot1->xAxis->setRange(start, end);
}

void MainWindow::setupWindowingVectors()
{
    const double PI = 2*acos(0);
    if(windowMult.size() > 0)
    {
        windowMult.clear();
    }
    switch(windowType)
    {
    case 1:
        for(int i = 0; i < numPoints; i++)
        {
            //with Compensation
            windowMult.push_back(2.3255814*(0.42659-0.49656*(cos((2*PI*i)/(numPoints-1)))+0.076849*(cos((4*PI*i)/(numPoints-1)))));
            double RBW = 60000000*1.69/numPoints;
            ui->RBW1->setText(QString::number(RBW));
            //Without Compensation
            //windowMult.push_back(0.42659-0.49656*(cos((2*PI*i)/(numPoints-1)))+0.076849*(cos((4*PI*i)/(numPoints-1))));
        }
        break;
    case 2:
        for(int i = 0; i < numPoints; i++)
        {
            //With Compensation
            windowMult.push_back(1-1.93*(cos((2*PI*i)/(numPoints-1)))+1.29*(cos((4*PI*i)/(numPoints-1)))-0.388*(cos((6*PI*i)/(numPoints-1)))+0.028*(cos((8*PI*i)/(numPoints-1))));

            double RBW = 60000000*3.77/numPoints;
            ui->RBW1->setText(QString::number(RBW));
            //Without Compensation: Not sure why we dont need to add compensation for this. It should be 4.54545455
        }
        break;
    case 3:
        for(int i = 0; i < numPoints; i++)
        {
            //With Compensation
            windowMult.push_back(2*0.5*(1-cos((2*PI*i)/(numPoints-1))));
            double RBW = 60000000*1.5/numPoints;
            ui->RBW1->setText(QString::number(RBW));
            //Without Compensation
            //windowMult.push_back(0.5*(1-cos((2*PI*i)/(numPoints-1))));
        }
        break;
    case 4:
        for(int i = 0; i < numPoints; i++)
        {
            //With Compensation
            windowMult.push_back(1.85185185*(0.54-0.46*(cos((2*PI*i)/(numPoints-1)))));
            double RBW = 60000000*1.36/numPoints;
            ui->RBW1->setText(QString::number(RBW));
            //Without Compensation
            //windowMult.push_back(0.54-0.46*(cos((2*PI*i)/(numPoints-1))));
        }
        break;
    }
}

void MainWindow::stopStuff()
{
    dataTimer->stop();
    newThread->setStop(true);
    newThread->exit();
    ui->customPlot1->clearGraphs();
}

void MainWindow::startStuff()
{
    if (newThread->isRunning())
    {
        stopStuff();
        QThread::sleep(1);
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
    clearPoints();
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
            tmpfftPoints = createDataPoints();
            fftPoints.push_back(tmpfftPoints);
        }
    }

    if(fftPoints.size() > 0)
    {
        getPlotValues(fftPoints);
    }

    key = time.elapsed()/THOUSAND; // set key to the time that has elasped from the start in seconds

    if (key-lastPointKey > 0.006)
    {

        // add data to lines:
        if (xValue.size() == plotPoints.size() && plotPoints.size() > 0) {
            ui->customPlot1->graph(0)->setData(xValue, plotPoints);
        }

        // rescale value (vertical) axis to fit the current data:
        ui->customPlot1->graph(0)->rescaleValueAxis(true, true);

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
                    QString("%1 FPS, Total Data points: %2, number of vectors: %3, total size : %4")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot1->graph(0)->data()->size())
                    .arg(fftPoints.size())
                    .arg(points->size())
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
    double temp = ((.06-S)/2);
    double startIndex = (dPoints/.06)*temp;
    double temp2 = ((.06+S)/2);
    double endIndex = (dPoints/.06)*temp2;
    double xinc = 0;
    maxPoint = -2000;
    maxFrequency = 0;
    double shift = S/2;
    double offset = getOffset(CF);


    if (endIndex <= points[0].size())
    {
        for(int i = startIndex; i < endIndex; i++ )
        {
            xinc = i - startIndex;
            xHertz = ((CF-shift) + (xinc * (.06/dPoints)));
            if (cfMhz==1)
            {
                xHertz*=1000;
            }
            xValue.push_back(xHertz);
            double avgPoint = 0;
            if (points.size() > 1)
            {
                for (int j = 0; j < points.size(); j++)
                {
                    avgPoint += points[j].at(i);// + offset;
                    if(points[j].at(i) > maxPoint)
                    {
                        maxPoint = points[j].at(i);
                        if (xValue.size() >= i)
                        {
                            maxFrequency = xValue.at(i);
                        }
                        //ui->Peak_Pwr->setText(QString::number(maxPoint));
                        //                        if(i >=0 && i <= points[j].size())
                        //                        {
                        //                            //maxFrequency = xValue.at(i);
                        //                            //ui->Peak_Freq->setText(QString::number(maxFrequency));
                        //                        }
                    }
                }
                if (i == v1Index)
                {
                    maxFrequency1 = xValue.at(i);
                }
                if (i == v2Index)
                {
                    maxFrequency2 = points[0].at(i);
                }
                avgPoint = avgPoint/points.size();
                plotPoints.push_back(avgPoint);
            } else
            {
                QVector<double> singlePoints = points[0];
                if(singlePoints.at(i) > maxPoint)
                {
                    maxPoint = singlePoints.at(i);
                    if (xValue.size() >= i)
                    {
                        maxFrequency = xValue.at(i);
                    }
                }
                if (i == v1Index)
                {
                    maxFrequency1 = singlePoints.at(i);
                }
                if (i == v2Index)
                {
                    maxFrequency2 = singlePoints.at(i);
                }
                plotPoints.push_back(singlePoints.at(i));// + offset);
            }

        }
        ui->Peak_Pwr->setText(QString::number(maxPoint));
        ui->Peak_Freq->setText(QString::number(maxFrequency));
        ui->FQ1->setText(QString::number(maxFrequency1));
        ui->C2FQ1->setText(QString::number(maxFrequency2));
        double vertdelt = maxFrequency1 - maxFrequency2;
        ui->VertDeltBox->setText(QString::number(vertdelt));
    }

}

QVector<double> MainWindow::createDataPoints()
{
    cleanPoints.clear();
    int i;
    QVector<double> fftPoints;
    QVector<double> ffttemp1;
    double dPoints = numPoints;
    fftw_complex in[numPoints], out[numPoints];
    fftw_plan p;

    p = fftw_plan_dft_1d(numPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    if (windowMult.size() > 0)
    {
        if(points->size() > numPoints && windowMult.size() == numPoints) {
            for (i = 0; i < numPoints; i++)
            {
                std::complex<double> current = points->dequeue();
                cleanPoints.push_back(current);
                in[i][0] = (current.real() * windowMult.at(i)/2048);
                in[i][1] = (current.imag() * windowMult.at(i)/2048);
            }
        }
    } else {
        if(points->size() > numPoints) {
            for (i = 0; i < numPoints; i++)
            {
                std::complex<double> current = points->dequeue();
                cleanPoints.push_back(current);
                in[i][0] = (current.real()/2048);
                in[i][1] = (current.imag()/2048);
                double RBW = 60000000/numPoints;
                ui->RBW1->setText(QString::number(RBW));

            }
        }
    }

    fftw_execute(p);

    for (i = 0; i < dPoints; i++)
    {
        if (i < dPoints/2)
        {
            //Magnitude: Unit Volts (V)
            double V = ((out[i][0]*out[i][0] + out[i][1]*out[i][1])/(dPoints*dPoints));
            //Volts RMS
            //double VRMS = V/sqrt(2);
            //Power Watts
            //double Watts = V*V/2;
            //Output
            //double Ppp = V;
            double dBFS = 10*log10(V);
            //            if (ui->Mode1->currentText()=="V"){
            //                           ffttemp1.push_back(V);
            //                       }

            //                       else if(ui->Mode1->currentText()=="dBV"){
            //                           ffttemp1.push_back(dBFS);
            //                       }
            //                       else if (ui->Mode1->currentText() == "Vrms"){
            //                       ffttemp1.push_back(VRMS);
            //                       }
            //                       else if (ui->Mode1->currentText() == "Watts"){

            //                       ffttemp1.push_back(Watts);
            //                       }
            //                     //  else if(ui->Mode1->currentText()=="dBm"){
            //                       //    ffttemp1.push_back(dBFS);
            //                       //}
            isLinear ? ffttemp1.push_back(V) : ffttemp1.push_back(dBFS);
        } else
        {
            //Magnitude: Unit Volts (V)
            double V =((out[i][0]*out[i][0] + out[i][1]*out[i][1])/(dPoints*dPoints));
            //Volts  RMS
            //double VRMS = V/sqrt(2);
            //Output:
            //double Ppp = V;
            double dBFS = 10*log10(V);
            //            if (ui->Mode1->currentText()=="V"){
            //                           ffttemp1.push_back(V);
            //                       }

            //                       else if(ui->Mode1->currentText()=="dBV"){
            //                           ffttemp1.push_back(dBFS);
            //                       }
            //                       else if (ui->Mode1->currentText() == "Vrms"){
            //                       ffttemp1.push_back(VRMS);
            //                       }
            //                       else if (ui->Mode1->currentText() == "Watts"){

            //                       ffttemp1.push_back(Watts);
            //                       }
            //                     //  else if(ui->Mode1->currentText()=="dBm"){
            //                       //    ffttemp1.push_back(dBFS);
            //                       //}
            isLinear ? fftPoints.push_back(V) : fftPoints.push_back(dBFS);
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

int MainWindow::getIndexFromHertz(double hertz)
{
    double dPoints = numPoints;
    double temp = ((.06-S)/2);

    double v1 = hertz - CF + (S/2) + temp;
    double v2 = dPoints/.06;

    return v1*v2;
}

void MainWindow::on_FFT1_currentIndexChanged(int index)
{
    if(!firstRun)
    {
        endRunningThread();
        tempNumPoints = ui->FFT1->itemData(index).toInt();
        QThread::currentThread()->wait(1);
        if (tempNumPoints >= 256)
        {
            refreshPlotting();
        }
    }
}

void MainWindow::on_Span1_editingFinished()
{
    double tSpan = ui->Span1->text().toDouble();

    if (spanMhz == 0)
    {
        if( tSpan > .001 && tSpan <= 60)
        {
            S = tSpan / THOUSAND;
            setupGraph();
        }
        else
        {
            QMessageBox::about(this, "Incorrect Value", "Enter a value between 100 and 5970");
        }
    } else {
        if ( tSpan > 1 && tSpan <= 60000)
        {
            S = tSpan/MILLION;
            setupGraph();
        }
        else
        {
            QMessageBox::about(this, "Incorrect Value", "Enter a number between 1 and 60000");
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
    std::cout << tCF;
    if (tCF != CF)
    {
        if (cfMhz == 1)
        {
            if( tCF >= 100 && tCF <= 5970)
            {
                endRunningThread();
                CF = tCF / THOUSAND;
                refreshPlotting();
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
            }
            else
            {
                QMessageBox::about(this, "Incorrect Value", "Enter a number between .1 and 5.97");
            }
        }
    }

}


void MainWindow::on_AB1_editingFinished()
{
    double tAB = ui->AB1->text().toDouble();
    if (AB != tAB)
    {
        endRunningThread();
        AB = tAB;
        refreshPlotting();
    }
}



void MainWindow::on_CF2_currentTextChanged(const QString &arg1)
{
    if (arg1 != ui->FQ2->text())
    {
        if (arg1 == "MHz")
        {
            ui->CF1->setText(QString::number(CF*1000));
            cfMhz= 1;
            ui->FQ2->setText("MHz");
            ui->C2FQ2->setText("MHz");
            ui->customPlot1->xAxis->setLabel("MHz");
            ui->PeakFreqLabel->setText("MHz");


        }
        else
        {
            ui->CF1->setText(QString::number(CF));
            cfMhz = 0;
            ui->FQ2->setText("GHz");
            ui->C2FQ2->setText("GHz");
            ui->customPlot1->xAxis->setLabel("GHz");
            ui->PeakFreqLabel->setText("GHz");

        }
        refreshPlotting();
    }

}

void MainWindow::on_Span2_currentTextChanged(const QString &arg1)
{
    if (arg1 == "kHz")
    {
        spanMhz= 1;
    }
    else
    {
        spanMhz = 0;
    }
}

void MainWindow::on_WSize_currentIndexChanged(int index)
{
    if (windowType != index)
    {
        endRunningThread();
        windowType = index;
        refreshPlotting();
    }
}

void MainWindow::on_Theme1_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "Dark")
    {
        ui->customPlot1->axisRect()->setBackground(Qt::black);
        ui->customPlot1->graph(0)->setPen(QPen(QColor(224, 195, 30)));
    }
    else if (arg1 == "White")
    {
        ui->customPlot1->axisRect()->setBackground(Qt::white);
        ui->customPlot1->graph(0)->setPen(QPen(QColor(30, 119, 227)));
    }
}

void MainWindow::on_Grid1_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "On")
    {
        ui->customPlot1->yAxis->setVisible(true);
        ui->customPlot1->xAxis->setVisible(true);
    }
    else if (arg1 == "Off")
    {
        ui->customPlot1->yAxis->setVisible(false);
        ui->customPlot1->xAxis->setVisible(false);
    }
}

void MainWindow::on_Export_clicked()
{
    if (newThread->isFinished() && cleanPoints.size() >= numPoints)
    {
        QDateTime date(QDateTime::currentDateTime());
        QString dateString = date.toString();
        QString s = dateString.replace(QRegExp(" "), "_");

        ui->widget->show();
        ui->centralWidget->grab().save("Spectrum"+ s + ".png");

        QFile file(QDir::currentPath() + "/data" + s + ".csv");
        if (file.open(QFile::WriteOnly|QFile::Truncate))
        {
            QTextStream stream(&file);
            stream << "I" << "\t" << "Q" << "\n";

            for (int i = 0; i < numPoints; i++)
            {
                std::complex<double> point = cleanPoints.at(i);
                stream << QString::number(point.real()) << "\t" << QString::number(point.imag()) << "\n";
            }
        }

        file.close();

    } else {
        QMessageBox::about(this, "Error", "Please stop the application to export data!");
    }
}

void MainWindow::on_AVG1_currentTextChanged()
{
    int tAvg = ui->AVG1->currentText().toInt();

    if((tAvg > 0 && tAvg <= 10) && tAvg != numberOfAverages)
    {
        endRunningThread();
        numberOfAverages = tAvg;
        refreshPlotting();
    }

}

void MainWindow::on_Mode1_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "Linear")
    {
        isLinear = true;
        ui->customPlot1->yAxis->setRange(-0.001,0.15);
        ui->MP2->setText("V");
        ui->customPlot1->yAxis->setLabel("Volts");
        ui->C2MP2->setText("V");
        ui->PeakPwrLabel->setText("V");
        ui->VertDeltaLabel->setText("V");
    } else
    {
        isLinear = false;
        ui->customPlot1->yAxis->setRange(-120,0);
        ui->MP2->setText("dBm");
        ui->customPlot1->yAxis->setLabel("dBm");
        ui->C2MP2->setText("dBm");
        ui->PeakPwrLabel->setText("dBm");
        ui->VertDeltaLabel->setText("dBm");
    }


    //    if (arg1 == "V" || arg1 == "Vrms" || arg1 == "Watts" )
    //    {
    //        isLinear = true;
    //        ui->customPlot1->yAxis->setRange(-0.001,0.15);
    //        ui->MP2->setText("V");

    //    }

    //    else if (arg1 == "dBV" || arg1 == "dBM")
    //    {
    //        isLinear = false;
    //        ui->customPlot1->yAxis->setRange(-120,0);
    //        ui->MP2->setText("dBV");
    //    }

    //    ui ->MP2->setText(ui->Mode1->currentText());
    //    ui->customPlot1->yAxis->setLabel(ui->Mode1->currentText());


}

void MainWindow::on_Settings_clicked()
{
    if (ui->widget->isVisible() == false)
    {
        ui->widget->show();
    }
    else if (ui->widget->isVisible() == true)
    {
        ui->widget->hide();
    }
}

void MainWindow::on_w3close_clicked()
{
    ui->widget->hide();
}

void MainWindow::on_Cursor1_currentIndexChanged(int index)
{
    if (index == 0)
    {
        ui->customPlot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    } else
    {
        ui->customPlot1->setInteractions(NULL);
    }

}
