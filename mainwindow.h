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
    void createDataPoints(QVector<double> &fftPoints);
    static void doStuff();

private Q_SLOTS:
        void realtimeDataSlot();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
