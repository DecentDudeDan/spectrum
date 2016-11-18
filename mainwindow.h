#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void createDataPoints(std::complex<double> *points, QVector<double> &xValue, QVector<double> &fftPoints);
    void clearAndRecreateGraph();
    void doStuff();

private slots:
        void realtimeDataSlot();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
