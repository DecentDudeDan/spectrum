#include "libthread.h"

enum iodev { RX, TX };

libThread::libThread(int numPoints, double AB, double CF)
{
    mPoints = numPoints;
    mAB = AB;
    mCF = CF;
    stop = false;
    std::cout << "thread created on: " << QThread::currentThread() << std::endl;
}

libThread::~libThread()
{
    std::cout << "thread dying!" << std::endl;
}


bool libThread::getStop()
{
    return stop;
}

void libThread::setStop(bool value)
{
    stop = value;
}

void libThread::shutdown()
{
    printf("* Destroying buffers\n");
    if (rxbuf) { iio_buffer_destroy(rxbuf); }
    //    if (txbuf) { iio_buffer_destroy(txbuf); }

    printf("* Disabling streaming channels\n");
    if (rx0_i) { iio_channel_disable(rx0_i); }
    if (rx0_q) { iio_channel_disable(rx0_q); }
    //    if (tx0_i) { iio_channel_disable(tx0_i); }
    //    if (tx0_q) { iio_channel_disable(tx0_q); }

    printf("* Destroying context\n");
    if (ctx) { iio_context_destroy(ctx); }
}

void libThread::handle_sig(int sig)
{
    printf("Waiting for process to finish...\n");
}

/* check return value of attr_write function */
void libThread::errchk(int v, const char* what) {
    if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
void libThread::wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
    errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
void libThread::wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
    errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
char* libThread::get_ch_name(const char* type, int id)
{
    snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
    return tmpstr;
}

/* returns ad9361 phy device */
struct iio_device* libThread::get_ad9361_phy(struct iio_context *ctx)
{
    struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
    assert(dev && "No ad9361-phy found");
    return dev;
}

/* finds AD9361 streaming IIO devices */
bool libThread::get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev)
{
    switch (d) {
    case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
    case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
    default: assert(0); return false;
    }
}

/* finds AD9361 streaming IIO channels */
bool libThread::get_ad9361_stream_ch(struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
    *chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
    if (!*chn)
        *chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
    return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
bool libThread::get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn)
{
    switch (d) {
    case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
    case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
    default: assert(0); return false;
    }
}

/* finds AD9361 local oscillator IIO configuration channels */
bool libThread::get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn)
{
    switch (d) {
    // LO chan is always output, i.e. true
    case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
    case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); return *chn != NULL;
    default: assert(0); return false;
    }
}

/* applies streaming configuration through IIO */
bool libThread::cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid)
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

//void libThread::run()
//{
//    std::cout << "in loop of thread: " << QThread::currentThread() << " and value of [numPoints, AB, CF] [" << mPoints << ", " << mAB << ", " << mCF << "]." << std::endl;
//    float rand_mag = 0;
//    float rand_freq = 0;
//    points->clear();
//    while(!stop)
//    {
//        if(points)
//        {
//            if(points->size() < mPoints * 10)
//            {
//                for(int i = 0; i < mPoints; i++)
//                {
//                    rand_mag = (qrand() % 10);
//                    rand_freq = (qrand() %100);
//                    double freq1 = 3;
//                    double real = (10*cos(2*3.14159265*freq1*i))+rand_mag*cos(2*3.14159265*rand_freq*i);//*rand_num;
//                    double img =  (10*sin(2*3.14159265*freq1*i))+rand_mag*sin(2*3.14159265*rand_freq*i);//*rand_num;
//                    points->enqueue({real, img});
//                }
//            }
//        }
//    }
//    qDebug() << "exiting run function";
//}

/* simple configuration and streaming */
void libThread::run()
{

    // Streaming devices
    //    struct iio_device *tx;
    struct iio_device *rx;

    // RX and TX sample counters
    size_t nrx = 0;
    size_t ntx = 0;

    // Stream configurations
    struct stream_cfg rxcfg;
    struct stream_cfg txcfg;

    // Listen to ctrl+c and assert
    signal(SIGINT, handle_sig);

    // RX stream config
    rxcfg.bw_hz = MHZ(mAB);   // value in AB for MHz rf bandwidth
    rxcfg.fs_hz = MHZ(60);   // 2.5 MS/s rx sample rate
    rxcfg.lo_hz = GHZ(mCF); // value in CF for GHz rf frequency
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

    //    // TX stream config
    //    txcfg.bw_hz = MHZ(30); // 1.5 MHz rf bandwidth
    //    txcfg.fs_hz = MHZ(mAB);   // 2.5 MS/s tx sample rate
    //    txcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
    //    txcfg.rfport = "A"; // port A (select for rf freq.)
    printf("* Acquiring IIO context\n");
    assert((ctx = iio_create_default_context()) && "No context");
    assert(iio_context_get_devices_count(ctx) > 0 && "No devices");

    printf("* Acquiring AD9361 streaming devices\n");
    //    assert(get_ad9361_stream_dev(ctx, TX, &tx) && "No tx dev found");
    assert(get_ad9361_stream_dev(ctx, RX, &rx) && "No rx dev found");

    printf("* Configuring AD9361 for streaming\n");
    assert(libThread::cfg_ad9361_streaming_ch(ctx, &rxcfg, RX, 0) && "RX port 0 not found");
    //    assert(libThread::cfg_ad9361_streaming_ch(ctx, &txcfg, TX, 0) && "TX port 0 not found");

    printf("* Initializing AD9361 IIO streaming channels\n");
    assert(libThread::get_ad9361_stream_ch(ctx, RX, rx, 2, &rx0_i) && "RX chan i not found");
    assert(libThread::get_ad9361_stream_ch(ctx, RX, rx, 3, &rx0_q) && "RX chan q not found");
    //    assert(libThread::get_ad9361_stream_ch(ctx, TX, tx, 0, &tx0_i) && "TX chan i not found");
    //    assert(libThread::get_ad9361_stream_ch(ctx, TX, tx, 1, &tx0_q) && "TX chan q not found");

    printf("* Enabling IIO streaming channels\n");
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);
    //    iio_channel_enable(tx0_i);
    //    iio_channel_enable(tx0_q);

    printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
    rxbuf = iio_device_create_buffer(rx, mPoints*2, false);
    if (!rxbuf) {
        perror("Could not create RX buffer");
        shutdown();
    }
    //    txbuf = iio_device_create_buffer(tx, mPoints*2, false);
    //    if (!txbuf) {
    //        perror("Could not create TX buffer");
    //        shutdown();
    //    }

    while(!stop)
    {
        ssize_t nbytes_rx, nbytes_tx;
        void *p_dat, *p_end, *p_dat_start;
        ptrdiff_t p_inc;

        // Schedule TX buffer
        //        nbytes_tx = iio_buffer_push(txbuf);
        //        if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

        // Refill RX buffer
        nbytes_rx = iio_buffer_refill(rxbuf);
        if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

        // READ: Get pointers to RX buf and read IQ from RX buf port 0
        p_inc = iio_buffer_step(rxbuf);
        p_end = iio_buffer_end(rxbuf);
        p_dat_start = iio_buffer_first(rxbuf, rx0_i);

        if (points && !(points->size() > mPoints*10)){
            for (p_dat = p_dat_start; p_dat < p_end-1; p_dat += p_inc) {
                const int i = (int)((int16_t*)p_dat)[0]; // Real (I)
                const int q = (int)((int16_t*)p_dat)[1]; // Imag (Q)
                //std::cout << "real: " << i << ", imag: " << q << std::endl;
                points->enqueue({i, q});
            }
        }
    }

shutdown();
}

