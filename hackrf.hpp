#ifndef HACKRF_H
#define HACKRF_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <libhackrf/hackrf.h>

#include <stdexcept>
#include <vector>
#include <complex>

typedef std::complex<float> complex_t;

template<class DEMOD>
class HackRF
{
public:

    HackRF();
    virtual ~HackRF();

    void open();

    void set_frequency( uint64_t freq );
    void set_sample_rate( uint64_t srate );
    void set_amp_enabled( bool enabled );
    void set_lna_gain( uint32_t gain );
    void set_vga_gain( uint32_t gain );

    void start();
    void stop();

    bool is_streaming() const;

protected:

    hackrf_device *_device;
    bool           _running;

    std::vector<complex_t> _lookup_table;

    DEMOD _demodulator;

    static int rx_callback(hackrf_transfer *transfer);
};

#define HACKRF_CHECK_STATUS(ret,message) \
    if ( ret != HACKRF_SUCCESS ) {\
        throw std::runtime_error(message); \
    }

template<class DEMOD>
HackRF<DEMOD>::HackRF() : _device(NULL), _running(false) {
    // create a lookup table for complex values
    _lookup_table.reserve( 0xffff + 1 );
    for( uint32_t i = 0; i <= 0xffff; ++i ) {
        _lookup_table.push_back(
            complex_t( (float(i & 0xff) - 127.5f) * (1.0f/128.0f),
                          (float(i >> 8) - 127.5f) * (1.0f/128.0f) )
        );
    }
}

template<class DEMOD>
void HackRF<DEMOD>::open() {
    int status = -1;

    status = hackrf_init();
    HACKRF_CHECK_STATUS( status, "Failed to initialize HackRf." );

    status = hackrf_open(&_device);
    HACKRF_CHECK_STATUS( status, "Failed to open device." );
}

template<class DEMOD>
void HackRF<DEMOD>::set_frequency( uint64_t freq ) {
    int status = hackrf_set_freq( _device, freq );
    HACKRF_CHECK_STATUS( status, "Failed to set frequency." );
}

template<class DEMOD>
void HackRF<DEMOD>::set_sample_rate( uint64_t srate ) {
    int status = hackrf_set_sample_rate( _device, srate );
    HACKRF_CHECK_STATUS( status, "Failed to set sample rate." );
}

template<class DEMOD>
void HackRF<DEMOD>::set_amp_enabled( bool enabled ) {
    int status = hackrf_set_amp_enable( _device, enabled ? 1 : 0 );
    HACKRF_CHECK_STATUS( status, "Failed to set AMP status." );
}

template<class DEMOD>
void HackRF<DEMOD>::set_lna_gain( uint32_t gain ) {
    int status = hackrf_set_lna_gain( _device, gain );
    HACKRF_CHECK_STATUS( status, "Failed to set LNA gain." );
}

template<class DEMOD>
void HackRF<DEMOD>::set_vga_gain( uint32_t gain ) {
    int status = hackrf_set_vga_gain( _device, gain );
    HACKRF_CHECK_STATUS( status, "Failed to set VGA gain." );
}

template<class DEMOD>
void HackRF<DEMOD>::start() {
    int status = hackrf_start_rx( _device, rx_callback, this );
    HACKRF_CHECK_STATUS( status, "Failed to start RX streaming." );
    _running = true;
}

template<class DEMOD>
void HackRF<DEMOD>::stop() {
    _running = false;
    hackrf_stop_rx(_device);
}

template<class DEMOD>
bool HackRF<DEMOD>::is_streaming() const {
    return ( hackrf_is_streaming(_device) == HACKRF_TRUE );
}

template<class DEMOD>
HackRF<DEMOD>::~HackRF() {
    if( _device ) {
        stop();
        hackrf_close(_device);
    }

    hackrf_exit();
}

template<class DEMOD>
int HackRF<DEMOD>::rx_callback(hackrf_transfer *transfer) {
    HackRF *hrf = (HackRF *)transfer->rx_ctx;

    if( hrf->_running ) {
        unsigned short *p = (unsigned short *)transfer->buffer;
        size_t i, len = transfer->valid_length / sizeof(unsigned short);

        std::vector<complex_t> values( len );
        for( i = 0; i < len; ++i ){
            values.push_back( hrf->_lookup_table[ p[i] ] );
        }

        hrf->_demodulator.demodulate(values);
    }

    return 0;
}

#endif
