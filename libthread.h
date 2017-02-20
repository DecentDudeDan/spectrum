#ifndef LIBTHREAD_H
#define LIBTHREAD_H

#include <iostream>
#include <math.h>
#include <QVector>
#include <QThread>
#include <QDebug>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fstream>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>
#include <ctime>
#include "concurrentqueue.h"

#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

extern ConcurrentQueue* points;

class libThread : public QThread
{
    Q_OBJECT

private:

double mCF;
double mAB;
bool stop;
bool valueChanged;
int mPoints;

/* RX is input, TX is output */
enum iodev { RX, TX };


/* static scratch mem for strings */
char tmpstr[64];

/* IIO structs required for streaming */
struct iio_context *ctx;
struct iio_channel *rx0_i;
struct iio_channel *rx0_q;
struct iio_channel *tx0_i;
struct iio_channel *tx0_q;
struct iio_buffer  *rxbuf;
struct iio_buffer  *txbuf;

public:

libThread(int numPoint, double AB, double CF);
virtual ~libThread();

struct stream_cfg {
    long long bw_hz; // Analog banwidth in Hz
    long long fs_hz; // Baseband sample rate in Hz
    long long lo_hz; // Local oscillator frequency in Hz
    const char* rfport; // Port name
};

    void shutdown();
    static void handle_sig(int sig);
    void errchk(int v, const char* what);
    void wr_ch_lli(struct iio_channel *chn, const char* what, long long val);
    void wr_ch_str(struct iio_channel *chn, const char* what, const char* str);
    char* get_ch_name(const char* type, int id);
    struct iio_device* get_ad9361_phy(struct iio_context *ctx);
    bool get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev);
    bool get_ad9361_stream_ch(struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn);
    bool get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn);
    bool get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn);
    bool cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid);

    void run();
    bool getStop();
    void setStop(bool value);
};

#endif // LIBTHREAD_H
