/*
 * Copyright (c) 2015, Simone Margaritelli <evilsocket at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of HackRF++ nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef HACKRF_H
#define HACKRF_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <libhackrf/hackrf.h>

#include <stdexcept>
#include <vector>

#include "iqreader.hpp"

template<class DEMOD>
class HackRFPP
{
public:

    HackRFPP();
    virtual ~HackRFPP();

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
    iq_reader      _iq_reader;
    DEMOD          _demodulator;

    static int rx_callback(hackrf_transfer *transfer);
};

#define HACKRF_CHECK_STATUS(ret,message) \
    if ( ret != HACKRF_SUCCESS ) {\
        throw std::runtime_error(message); \
    }

template<class DEMOD>
HackRFPP<DEMOD>::HackRFPP() : _device(NULL), _running(false) {

}

template<class DEMOD>
void HackRFPP<DEMOD>::open() {
    int status = -1;

    status = hackrf_init();
    HACKRF_CHECK_STATUS( status, "Failed to initialize HackRf." );

    status = hackrf_open(&_device);
    HACKRF_CHECK_STATUS( status, "Failed to open device." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::set_frequency( uint64_t freq ) {
    int status = hackrf_set_freq( _device, freq );
    HACKRF_CHECK_STATUS( status, "Failed to set frequency." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::set_sample_rate( uint64_t srate ) {
    int status = hackrf_set_sample_rate( _device, srate );
    HACKRF_CHECK_STATUS( status, "Failed to set sample rate." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::set_amp_enabled( bool enabled ) {
    int status = hackrf_set_amp_enable( _device, enabled ? 1 : 0 );
    HACKRF_CHECK_STATUS( status, "Failed to set AMP status." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::set_lna_gain( uint32_t gain ) {
    int status = hackrf_set_lna_gain( _device, gain );
    HACKRF_CHECK_STATUS( status, "Failed to set LNA gain." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::set_vga_gain( uint32_t gain ) {
    int status = hackrf_set_vga_gain( _device, gain );
    HACKRF_CHECK_STATUS( status, "Failed to set VGA gain." );
}

template<class DEMOD>
void HackRFPP<DEMOD>::start() {
    int status = hackrf_start_rx( _device, rx_callback, this );
    HACKRF_CHECK_STATUS( status, "Failed to start RX streaming." );
    _running = true;
}

template<class DEMOD>
void HackRFPP<DEMOD>::stop() {
    _running = false;
    hackrf_stop_rx(_device);
}

template<class DEMOD>
bool HackRFPP<DEMOD>::is_streaming() const {
    return ( hackrf_is_streaming(_device) == HACKRF_TRUE );
}

template<class DEMOD>
HackRFPP<DEMOD>::~HackRFPP() {
    if( _device ) {
        stop();
        hackrf_close(_device);
    }

    hackrf_exit();
}

template<class DEMOD>
int HackRFPP<DEMOD>::rx_callback(hackrf_transfer *transfer) {
    HackRFPP *hrf = (HackRFPP *)transfer->rx_ctx;

    if( hrf->_running ) {
        hrf->_demodulator.demodulate(
            hrf->_iq_reader.parse( transfer->buffer, transfer->valid_length )
        );
    }

    return 0;
}

#endif
