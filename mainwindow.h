#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>
#include <QtConcurrent/QtConcurrent>


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
    static void doStuff();
    void startStuff();
    void stopStuff();

private Q_SLOTS:
        void realtimeDataSlot();
        void on_startButton_clicked();
        void on_FFT1_currentIndexChanged(int index);
        void on_StopButton_clicked();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
