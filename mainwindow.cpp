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
    maxPoint(-200),
    cfMhz(0),
    spanMhz(0),
    firstRun(true),
    isLinear(false)
{
    dataTimer = new QTimer();

    ui->setupUi(this);

    newThread = new libThread(numPoints, AB, CF);

    setupGraph();
    setGUIValues();
    ui->widget->show();

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
    ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)2);


//if (ui->CF2->currentText() == "GHz")
    ui->customPlot1->xAxis->setLabel("GHz");
//else
    //ui->customPlot1->xAxis->setLabel("MHz");
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
       // ui->customPlot1->yAxis->
        ui->customPlot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
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
        ui->customPlot1->yAxis->setRange(-120, 0);
    } else
    {
        ui->customPlot1->yAxis->setRange(-0.001,0.15);
    }








    setupWindowingVectors();
}
//////Cursors Attempt 1
//typedef struct {
//        QCPItemLine *hLine;
//        QCPItemLine *vLine;
//}QCPCursor;

//bool cursorEnabled=true;

//void ManageCursor(QCustomPlot *customPlot, QCPCursor *cursor, double x, double y, QPen pen)
//{
//        if(cursorEnabled)
//{
//    if(cursor->hLine) customPlot->removeItem(cursor->hLine);
//    cursor->hLine = new QCPItemLine(customPlot);
//    customPlot->addGraph();
//    customPlot->graph(0)->setData(0, cursor->hLine);
//    cursor->hLine->setPen(pen);
//    cursor->hLine->start->setCoords(QCPRange::minRange,y);
//    cursor->hLine->end->setCoords(QCPRange::maxRange, y);

//    if(cursor->vLine) customPlot->removeItem(cursor->vLine);
//    cursor->vLine = new QCPItemLine(customPlot);
//    customPlot->addGraph(cursor->vLine);
//    cursor->vLine->setPen(pen);
//    cursor->vLine->start->setCoords( x, QCPRange::minRange);
//    cursor->vLine->end->setCoords( x, QCPRange::maxRange);
//}


//}

//void MainWindow::mouseRelease(QMouseEvent* event)
//{
//    QCustomPlot *customPlot=ui->customPlot1;
//    static QCPCursor cursor1, cursor2;
//    double x=customPlot1->xAxis->pixelToCoord(event->pos().x());
//    double y=customPlot1->yAxis->pixelToCoord(event->pos().y());
//    if(event->button() == Qt::LeftButton)
//        ManageCursor(customPlot1, &cursor1, x,y, QPen(Qt::black));
//    else
//        ManageCursor(customPlot1, &cursor2, x, y, QPen(Qt::white));
//    customPlot1->replot();
//    cursorEnabled=true;


//}
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
            windowMult.push_back(0.42659-0.49656*(cos((2*PI*i)/(numPoints-1)))+0.076849*(cos((4*PI*i)/(numPoints-1))));
        }
        break;
    case 2:
        for(int i = 0; i < numPoints; i++)
        {
            windowMult.push_back(1-1.93*(cos((2*PI*i)/(numPoints-1)))+1.29*(cos((4*PI*i)/(numPoints-1)))-0.388*(cos((6*PI*i)/(numPoints-1)))+0.028*(cos((8*PI*i)/(numPoints-1))));
        }
        break;
    case 3:
        for(int i = 0; i < numPoints; i++)
        {
            windowMult.push_back(0.5*(1-cos((2*PI*i)/(numPoints-1))));
        }
        break;
    case 4:
        for(int i = 0; i < numPoints; i++)
        {
            windowMult.push_back(0.54-0.46*(cos((2*PI*i)/(numPoints-1))));
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
                    QString("%1 FPS, Total Data points: %2, number of vectors: %3, plotPoints: %4, xValues: %5, max Point Overall: %6")
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
    double temp = ((.06-S)/2);
    double startIndex = (dPoints/.06)*temp;
    double temp2 = ((.06+S)/2);
    double endIndex = (dPoints/.06)*temp2;
    double xinc = 0;
    maxPoint = -2000;
    maxFrequency = 0;
    double shift = S/2;


    if (endIndex <= points[0].size())
    {
        for(int i = startIndex; i < endIndex; i++ )
        {
            xinc = i - startIndex;
            xHertz = ((CF-shift) + (xinc * (.06/dPoints)));
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
                        if (i >= 0 && i <= points[j].size())
                        {
                            //maxFrequency = xValue.at(i);
                        }
                    }
                }
                avgPoint = avgPoint/points.size();
                plotPoints.push_back(avgPoint);
            } else
            {
                if(points[0].at(i) > maxPoint)
                {
                    maxPoint = points[0].at(i);
                    if (i >= 0 && i <= points[0].size())
                    {
                        //maxFrequency = xValue.at(i);
                    }
                }
                plotPoints.push_back(points[0].at(i));
            }


        }

        ui->MP1->setText(QString::number(maxPoint));
        ui->FQ1->setText(QString::number(maxFrequency));
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
                in[i][0] = (current.real() * windowMult.at(i));
                in[i][1] = (current.imag() * windowMult.at(i));
            }
        }
    } else {
        if(points->size() > numPoints) {
            for (i = 0; i < numPoints; i++)
            {
                std::complex<double> current = points->dequeue();
                cleanPoints.push_back(current);
                in[i][0] = current.real();
                in[i][1] = current.imag();
            }
        }
    }

    fftw_execute(p);

    for (i = 0; i < dPoints; i++)
    {
        if (i < dPoints/2)
        {
            //Magnitude: Unit Volts (V)
            double V = (sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]))/(dPoints);
            //Volts RMS
            double VRMS = V/sqrt(2);
            //Power Watts
            double Watts = V*V/2;
            //Output
            double Ppp = V;
            double dBFS = 20*log10(VRMS);

            isLinear ? ffttemp1.push_back(Ppp) : ffttemp1.push_back(dBFS);
        } else
        {
             //Magnitude: Unit Volts (V)
            double V = (sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]))/(dPoints);
            //Volts  RMS
            double VRMS = V/sqrt(2);
            //Power Watts
            double Watts = V*V/2;
            //Output:
            double Ppp = Watts;
            double dBFS = 20*log10(VRMS);

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


void MainWindow::on_AB1_editingFinished()
{
    double tAB = ui->AB1->text().toDouble();
    if (tAB)
    {
        tempAB = tAB;
    }
}



void MainWindow::on_CF2_currentTextChanged(const QString &arg1)
{
    if (arg1 == "MHz")
    {
        cfMhz= 1;
        ui->FQ2->setText("MHz");
    }
    else
    {
        cfMhz = 0;
        ui->FQ2->setText("GHz");
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
    endRunningThread();
    windowType = index;
    refreshPlotting();
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

void MainWindow::on_AVG1_currentTextChanged(const QString &arg1)
{
    int tAvg = ui->AVG1->currentText().toInt();

    if(tAvg > 0 && tAvg <= 10)
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
    } else
    {
        isLinear = false;
        ui->customPlot1->yAxis->setRange(-120,0);
        ui->MP2->setText("dBV");
    }

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
