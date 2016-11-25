#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtimer.h"
#include<QString>
#include <fftw3.h>
#include <iostream>
#include <math.h>
#include <QVector>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>
#include <ctime>
#include "concurrentqueue.h"
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

int cf;
int ab;
int numPoints;

/* RX is input, TX is output */
enum iodev { RX, TX };

/* common RX and TX streaming params */
struct stream_cfg {
    long long bw_hz; // Analog banwidth in Hz
    long long fs_hz; // Baseband sample rate in Hz
    long long lo_hz; // Local oscillator frequency in Hz
    const char* rfport; // Port name
};

/* static scratch mem for strings */
static char tmpstr[64];

/* IIO structs required for streaming */
static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
static struct iio_buffer  *txbuf = NULL;

static bool stop;

ConcurrentQueue points;
QVector<double> xValue;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->customPlot1->setBackground(Qt::lightGray);
    ui->customPlot1->axisRect()->setBackground(Qt::black);

    ui->customPlot2->setBackground(Qt::lightGray);
    ui->customPlot2->axisRect()->setBackground(Qt::black);


    // add a graph to the plot and set it's color to blue:
    ui->customPlot1->addGraph();
    ui->customPlot1->graph(0)->setPen(QPen(QColor(224, 195, 30)));
    ui->customPlot1->graph(0)->setLineStyle((QCPGraph::LineStyle)2);

    ui->customPlot2->addGraph();
    ui->customPlot2->graph(0)->setPen(QPen(QColor(40, 255, 255)));
    ui->customPlot2->graph(0)->setLineStyle((QCPGraph::LineStyle)1);

    // set x axis to be a time ticker and y axis to be from -1.5 to 1.5:
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot1->xAxis->setRange(0,numPoints);
    ui->customPlot1->axisRect()->setupFullAxesBox();
    ui->customPlot1->yAxis->setRange(1, 3000000);
    ui->customPlot1->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot1->yAxis->setTicker(logTicker);

    ui->customPlot2->xAxis->setTicker(timeTicker);
    ui->customPlot2->axisRect()->setupFullAxesBox();
    ui->customPlot2->yAxis->setRange(-1.5, 1.5);

    // make left and bottom axes transfer their ranges to right and top axes:

    connect(ui->customPlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->yAxis2, SLOT(setRange(QCPRange)));

    QFuture<void> future = QtConcurrent::run(doStuff);

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot when the timer times out:
    QTimer *dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(ui->StopButton, SIGNAL(clicked()),dataTimer, SLOT(stop()));
    connect(ui->startButton, SIGNAL(clicked()),dataTimer, SLOT(start()));

    //setup user inputs
    ui->FFT1->addItem("512", QVariant(512));
    ui->FFT1->addItem("1024", QVariant(1024));
    ui->FFT1->addItem("2048", QVariant(2048));
    ui->FFT1->addItem("4096", QVariant(4096));
    ui->FFT1->addItem("8192", QVariant(8192));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::realtimeDataSlot()
{
    cf = ui->CF->text().toInt();
    ab = ui->AB->text().toInt();

    static QTime time(QTime::currentTime());
    QVector<double> fftPoints;
    double key;
    static double lastPointKey;

    // generate a vector of double's for the x Axis
    if(xValue.isEmpty()) {
        for(int i = 0; i < numPoints; i++) {
            xValue.push_back(i);
        }
    }

    if(points.size() > numPoints)  {
        fftPoints = createDataPoints();
    }

    key = time.elapsed()/1000.0; // set key to the time that has elasped from the start in seconds

    if (key-lastPointKey > 0.002)
    {
        // add data to lines:
        if (xValue.size() == fftPoints.size() && fftPoints.size() == numPoints) {
            ui->customPlot1->graph(0)->setData(xValue, fftPoints);
        }
        // TODO: add event for changing graph color based on value of graph.

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
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot1->graph(0)->data()->size())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }

}

QVector<double> MainWindow::createDataPoints()
{
    int i;
    QVector<double> fftPoints;
    fftw_complex in[numPoints], out[numPoints];
    fftw_plan p;

    p = fftw_plan_dft_1d(numPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    if(points.size() > numPoints) {
        for (i = 0; i < numPoints; i++)
        {
            std::complex<double> current = points.dequeue();
            in[i][0] = current.real();
            in[i][1] = current.imag();
            points.unlock();

        }
    }

    fftw_execute(p);

    for (i = 0; i < numPoints; i++)
    {
        fftPoints.push_back(sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]));
    }

    fftw_destroy_plan(p);

    return fftPoints;

}

/* cleanup and exit */
static void shutdown()
{
    printf("* Destroying buffers\n");
    if (rxbuf) { iio_buffer_destroy(rxbuf); }
    if (txbuf) { iio_buffer_destroy(txbuf); }

    printf("* Disabling streaming channels\n");
    if (rx0_i) { iio_channel_disable(rx0_i); }
    if (rx0_q) { iio_channel_disable(rx0_q); }
    if (tx0_i) { iio_channel_disable(tx0_i); }
    if (tx0_q) { iio_channel_disable(tx0_q); }

    printf("* Destroying context\n");
    if (ctx) { iio_context_destroy(ctx); }
    exit(0);
}

static void handle_sig(int sig)
{
    printf("Waiting for process to finish...\n");
    stop = true;
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
    if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
    errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
    errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
    snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
    return tmpstr;
}

/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(struct iio_context *ctx)
{
    struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
    assert(dev && "No ad9361-phy found");
    return dev;
}

/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev)
{
    switch (d) {
    case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
    case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
    default: assert(0); return false;
    }
}

/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
    *chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
    if (!*chn)
        *chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
    return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn)
{
    switch (d) {
    case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
    case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
    default: assert(0); return false;
    }
}

/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn)
{
    switch (d) {
    // LO chan is always output, i.e. true
    case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
    case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); return *chn != NULL;
    default: assert(0); return false;
    }
}

/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid)
{
    struct iio_channel *chn = NULL;

    // Configure phy and lo channels
    printf("* Acquiring AD9361 phy channel %d\n", chid);
    if (!get_phy_chan(ctx, type, chid, &chn)) {	return false; }
    wr_ch_str(chn, "rf_port_select",     cfg->rfport);
    wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
    wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

    // Configure LO channel
    printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
    if (!get_lo_chan(ctx, type, &chn)) { return false; }
    wr_ch_lli(chn, "frequency", cfg->lo_hz);
    return true;
}

/* simple configuration and streaming */
void MainWindow::doStuff()
{

    // Streaming devices
    struct iio_device *tx;
    struct iio_device *rx;

    // RX and TX sample counters
    size_t nrx = 0;
    size_t ntx = 0;

    // Stream configurations
    struct stream_cfg rxcfg;
    struct stream_cfg txcfg;

    // Listen to ctrl+c and assert
    signal(SIGINT, handle_sig);

    // RX stream config (Modify these variables )


    rxcfg.bw_hz = MHZ(2);   // 2 MHz rf bandwidth
    rxcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s rx sample rate
    rxcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)



    // TX stream config
    txcfg.bw_hz = MHZ(2); // 1.5 MHz rf bandwidth
    txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
    txcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
    txcfg.rfport = "A"; // port A (select for rf freq.)

    printf("* Acquiring IIO context\n");
    assert((ctx = iio_create_default_context()) && "No context");
    assert(iio_context_get_devices_count(ctx) > 0 && "No devices");

    printf("* Acquiring AD9361 streaming devices\n");
    assert(get_ad9361_stream_dev(ctx, TX, &tx) && "No tx dev found");
    assert(get_ad9361_stream_dev(ctx, RX, &rx) && "No rx dev found");

    printf("* Configuring AD9361 for streaming\n");
    assert(cfg_ad9361_streaming_ch(ctx, &rxcfg, RX, 0) && "RX port 0 not found");
    assert(cfg_ad9361_streaming_ch(ctx, &txcfg, TX, 0) && "TX port 0 not found");

    printf("* Initializing AD9361 IIO streaming channels\n");
    assert(get_ad9361_stream_ch(ctx, RX, rx, 0, &rx0_i) && "RX chan i not found");
    assert(get_ad9361_stream_ch(ctx, RX, rx, 1, &rx0_q) && "RX chan q not found");
    assert(get_ad9361_stream_ch(ctx, TX, tx, 0, &tx0_i) && "TX chan i not found");
    assert(get_ad9361_stream_ch(ctx, TX, tx, 1, &tx0_q) && "TX chan q not found");

    printf("* Enabling IIO streaming channels\n");
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);
    iio_channel_enable(tx0_i);
    iio_channel_enable(tx0_q);

    printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    rxbuf = iio_device_create_buffer(rx, numPoints*2, false);
    if (!rxbuf) {
        perror("Could not create RX buffer");
        shutdown();
    }
    txbuf = iio_device_create_buffer(tx, numPoints*2, false);
    if (!txbuf) {
        perror("Could not create TX buffer");
        shutdown();
    }

    printf("* Starting IO streaming (press CTRL+C to cancel)\n");
    while (!stop)
    {
        ssize_t nbytes_rx, nbytes_tx;
        void *p_dat, *p_end, *p_dat_start;
        ptrdiff_t p_inc;

        // Schedule TX buffer
        nbytes_tx = iio_buffer_push(txbuf);
        if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

        // Refill RX buffer
        nbytes_rx = iio_buffer_refill(rxbuf);
        if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

        // READ: Get pointers to RX buf and read IQ from RX buf port 0
        p_inc = iio_buffer_step(rxbuf);
        p_end = iio_buffer_end(rxbuf);
        p_dat_start = iio_buffer_first(rxbuf, rx0_i);

        for (p_dat = p_dat_start; p_dat < p_dat_start+numPoints*p_inc; p_dat += p_inc) {
            const int i = (int)((int16_t*)p_dat)[0]; // Real (I)
            const int q = (int)((int16_t*)p_dat)[1]; // Imag (Q)
            //std::cout << "real: " << i << ", imag: " << q << std::endl;
            points.enqueue({i, q});
        }

        // WRITE: Get pointers to TX buf and write IQ to TX buf port 0
        p_inc = iio_buffer_step(txbuf);
        p_end = iio_buffer_end(txbuf);
        for (p_dat = iio_buffer_first(txbuf, tx0_i); p_dat < p_end; p_dat += p_inc) {
            // Example: fill with zeros
            ((int16_t*)p_dat)[0] = 300000; // Real (I)
            ((int16_t*)p_dat)[1] = 300000; // Imag (Q)
        }

        // Sample counter increment and status output
        nrx += nbytes_rx / iio_device_get_sample_size(rx);
        ntx += nbytes_tx / iio_device_get_sample_size(tx);
        //printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx/1e6, ntx/1e6);
    }

    shutdown();
}





void MainWindow::on_FFT1_currentIndexChanged(int index)
{
    numPoints = ui->FFT1->itemData(index).toInt();
}
