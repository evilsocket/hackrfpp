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
#include <string.h>

#include <iostream>

#include "ask.hpp"
#include "hackrfpp.hpp"
#include "bitstream.hpp"

#define BUF_LEN     ( 16 * 32 * 512 )
#define FREQUENCY   ( 13.56 * 1e6 )
#define SAMPLE_RATE ( 8 * 1e6 )
//  ~106 kbit/s - http://jpkc.szpt.edu.cn/2007/sznk/UploadFile/biaozhun/iso14443/14443-2.pdf
#define BITRATE     ( FREQUENCY / 128.f )

struct ByteEmitter {
    static void emit( uint8_t byte ) {
        printf( "%02x ", byte );
        fflush( stdout );
    }
};

ask demod;

struct AM {
    static void demodulate( const std::vector<complex_t>& data ) {
        bitstream<ByteEmitter> stream;
        double prev_mag = 0;

        for( std::vector<complex_t>::const_iterator i = data.begin(), e = data.end(); i != e; ++i ){
            const complex_t &c = *i;

            // scale magnitude in the interval [-1.0, ~1.0] ( 0.984406 )
            double magnitude = std::norm(c) - 1;

            // uint8_t cur_bit = demod.demodulate(c);

            // manchester coding, low-to-high = 0, high-to-low = 1
            if( prev_mag < magnitude ) {
                stream << 1;
            }
            else if( prev_mag > magnitude ) {
                stream << 0;
            }
            else {
                printf( "\n");
            }

            prev_mag = magnitude;

            // stream << bit;
            // unsigned int symbol = demod.demodulate(c);

            // printf( "%02x ", symbol );
            // fflush( stdout );

            // scale magnitude in the interval [-1.0, ~1.0] ( 0.984406 )
            // double magnitude = std::norm(c) - 1;

            // This is WRONG! Still need to figure out how to get bits out of this.
            // uint8_t bit = magnitude <= 0 ? 0 : 1;

            // stream << bit;
        }
    }
};

static bool stopped = false;

void signal_handler(int dummy) {
    printf( "\n@ Received CTRL+C\n" );

    stopped = true;
}

int main( int argc, char **argv )
{
    signal(SIGINT, signal_handler);

    if( argc == 3 && strcmp( argv[1], "--file" ) == 0 ){
        std::cout << "Using IQ file " << argv[2] << " as input." << std::endl;

        iq_reader iqreader;

        FILE *fp = fopen( argv[2], "rb" );
        if( fp ){
            uint8_t buffer[BUF_LEN] = {0};

            while( !feof(fp) && fread( buffer, BUF_LEN, 1, fp ) == 1 && !stopped ){
                AM::demodulate( iqreader.parse( buffer, BUF_LEN ) );
            }

            fclose(fp);
        }
        else {
            std::cerr << "ERROR: Could not open file." << std::endl;
        }
    }
    else {

        std::cout << "Using HackRF device as input." << std::endl;

        try
        {
            HackRFPP<AM> dev;

            dev.open();

            dev.set_frequency( FREQUENCY );
            dev.set_sample_rate( SAMPLE_RATE );
            dev.set_amp_enabled( false );
            dev.set_lna_gain( 32 );
            dev.set_vga_gain( 30 );

            dev.start();

            while( stopped == false && dev.is_streaming() ) {
                usleep(100);
            }
        }
        catch( const std::exception &e ) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
    }

    return 0;
}
