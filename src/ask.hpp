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
#ifndef ASK_H
#define ASK_H

#include "iqlookup.hpp"

#define BITS_PER_SYMBOL 1

#define ASK8_ALPHA (1./sqrt(21))
#define ASK256_ALPHA    (1./sqrt(21845))

class ask {
protected:

    // Reference vector for demodulating linear arrays.
    float _ref[1];

    // Demodulate a linear symbol constellation using refereneced lookup table.
    unsigned int demod_linear( float real, float& residual ) {
        unsigned int i, s = 0;

        for( i = 0; i < BITS_PER_SYMBOL; ++i ) {
            // prepare symbol for next demodulated bit
            s <<= 1;
            // compare received value to zero
            if ( real > 0 ) {
                // shift '1' into symbol, subtract reference
                s |= 1;
                real -= _ref[ BITS_PER_SYMBOL - i - 1 ];
            } else {
                // shift '0' into symbol, add reference
                s |= 0;
                real += _ref[ BITS_PER_SYMBOL - i - 1 ];
            }
        }

        residual = real;

        return s;
    }

public:

    ask() {
        for( int i = 0; i < BITS_PER_SYMBOL; ++i ) {
            _ref[i] = ( 1 << i );
        }
    }

    unsigned int demodulate( const complex_t& c ) {
        float resid;

        unsigned int sym = demod_linear( c.real(), resid );
        // gray encoding
        sym = sym ^ (sym >> 1);

        return sym;
    }
};

#endif
