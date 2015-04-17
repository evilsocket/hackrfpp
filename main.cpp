#include <stdio.h>
#include <signal.h>
#include <iostream>
#include "hackrf.hpp"

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

HackRF<AM> hrf;

void signal_handler(int dummy) {
    printf( "\n@ Received CTRL+C\n" );

    hrf.stop();
}

int main( int argc, char **argv )
{
    signal(SIGINT, signal_handler);

    try
    {
        hrf.open();

        hrf.set_frequency( 13.56 * 1e6 );
        hrf.set_sample_rate( 8 * 1e6 );
        hrf.set_amp_enabled( false );
        hrf.set_lna_gain( 32 );
        hrf.set_vga_gain( 30 );

        hrf.start();

        while( hrf.is_streaming() ) {
            usleep(100);
        }
    }
    catch( const std::exception &e ) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}
