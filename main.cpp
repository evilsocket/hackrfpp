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

#define BUF_LEN (16 * 32 * 512)

class bitstream {
private:
    uint8_t _nbit;
    uint8_t _byte;

    void reset() {
        _nbit = 0;
        _byte = 0;
    }

public:

    bitstream() {
        reset();
    }

    void collect_bit( uint8_t bit ) {
        _byte |= ( bit << _nbit );
        ++_nbit;
        // printf( "%d\n", _nbit );
        if( _nbit >= 8 ) {
            printf( "%02x ", _byte );
            fflush(stdout);
            reset();
        }
    }
};

struct AM {
    static void demodulate( const std::vector<complex_t>& data ) {
        bitstream stream;

        for( std::vector<complex_t>::const_iterator i = data.cbegin(), e = data.cend(); i != e; ++i ){
            const complex_t &IQ = *i;

            double magnitude = std::norm(IQ);
            if( magnitude == 0 ){
                continue;
            }

            uint8_t bit = magnitude < 1 ? 0 : 1;
        
            stream.collect_bit( bit );

            // printf( "%c", magnitude < 0.8 ? '_' : '-' );
            // fflush( stdout );
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

    if( argc == 3 && strcmp( argv[1], "--file" ) == 0 ){
        std::cout << "Using IQ file " << argv[2] << " as input." << std::endl;

        FILE *fp = fopen( argv[2], "rb" );
        if( fp ){
            uint8_t buffer[BUF_LEN] = {0};

            while( !feof(fp) && fread( buffer, BUF_LEN, 1, fp ) == 1 ){
                unsigned short *p = (unsigned short *)&buffer;
                size_t i, len = BUF_LEN / sizeof(unsigned short);

                std::vector<complex_t> values( len );
                for( i = 0; i < len; ++i ){
                    values.push_back( dev.lookup( p[i] ) );
                }

                AM::demodulate(values);
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
    }

    return 0;
}
