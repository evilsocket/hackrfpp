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
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include "hackrfpp.hpp"

struct AM {
    void demodulate( const std::vector<complex_t>& data ) {
        for( std::vector<complex_t>::const_iterator i = data.cbegin(), e = data.cend(); i != e; ++i ){
            const complex_t &IQ = *i;

            double magnitude = sqrt( IQ.real() * IQ.real() + IQ.imag() * IQ.imag() );

            if( magnitude < 1.0 ){
                printf( "%c", magnitude < 0.8 ? '_' : '-' );

            }
            else {
                printf( " " );
            }
            fflush( stdout );
        }
    }
};

HackRFPP<AM> dev;

void signal_handler(int dummy) {
    printf( "\n@ Received CTRL+C\n" );

    dev.stop();
}

int main( int argc, char **argv )
{
    signal(SIGINT, signal_handler);

    try
    {
        dev.open();

        dev.set_frequency( 13.56 * 1e6 );
        dev.set_sample_rate( 8 * 1e6 );
        dev.set_amp_enabled( false );
        dev.set_lna_gain( 32 );
        dev.set_vga_gain( 30 );

        dev.start();

        while( dev.is_streaming() ) {
            usleep(100);
        }
    }
    catch( const std::exception &e ) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}
