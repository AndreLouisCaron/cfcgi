// Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include <fcgi.h>

#include <ctime>
#include <iostream>
#include <string>

namespace {

    const char DATA1[] =
    {
        // head.
        1,    // version       : FCGI_VERSION_1
        1,    // type          : FCGI_BEGIN_REQUEST
        0, 1, // request id    : 1
        0, 1, // content length: 8
        0,    // padding       : 0
        0,    // reserved      : ...

        // body.
        0, 1,          // role : FCGI_RESPONDER
        0,             // flags: 0
        0, 0, 0, 0, 0, // reserved: ...
    };
    const size_t SIZE1 = sizeof(DATA1);

    const char DATA2[] =
    {
        // head.
        1,    // version       : FCGI_VERSION_1
        2,    // type          : FCGI_ABORT_REQUEST
        0, 1, // request id    : 1
        0, 0, // content length: 0
        0,    // padding       : 1
        0,    // reserved      : ...
        
        // body.
        //0,
    };
    const size_t SIZE2 = sizeof(DATA2);

    const char DATA3[] =
    {
        // head.
        1,     // version       : FCGI_VERSION_1
        5,     // type          : FCGI_STDIN
        0,  1, // request id    : 1
        0, 13, // content length: 13
        0,     // padding       : 0
        0,     // reserved      : ...
        
        // body,
        //'h', 'e', 'l', 'l', 'o'
        "hello, world!",
    };
    const size_t SIZE3 = sizeof(DATA3)-1;

    const char DATA4[] =
    {
        // head.
        1,     // version       : FCGI_VERSION_1
        4,     // type          : FCGI_PARAM
        0,  1, // request id    : 1
        0, 15, // content length: 15
        9,     // padding       : 9
        0,     // reserved      : ...
        
        // body,
        11, // name length: 11
         2, // data length:  2
        'S','E','R','V','E','R','_','P','O','R','T', // name.
        '8','0',                                     // data.
        
        // pads.
        0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    const size_t SIZE4 = sizeof(DATA4);

    void accept_record
        ( ::fcgi_iwire * stream, int version, int request, int content )
    {
        std::cerr
            << "Accept record."
            << std::endl;
        if ( version != 1 ) {
            std::cerr << "   - invalid version." << std::endl;
        }
        if ( request != 1 ) {
            std::cerr << "   - invalid request id." << std::endl;
        }
    }

    void finish_record ( ::fcgi_iwire * stream )
    {
        std::cerr
            << "Finish record."
            << std::endl;
    }

    void accept_request ( ::fcgi_iwire * stream, int role, int flags )
    {
        std::cerr
            << "Accept request."
            << std::endl;
        if ( role != 1 ) {
            std::cerr << "   - invalid role." << std::endl;
        }
        if ( flags != 0 ) {
            std::cerr << "   - invalid flags." << std::endl;
        }
    }

    void cancel_request ( ::fcgi_iwire * stream )
    {
        std::cerr
            << "Cancel request."
            << std::endl;
    }

    void accept_content_stdi
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        std::cout
            << "stdin: '" << std::string(data, size) << "'."
            << std::endl;
    }

    void accept_param_name
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        std::cout
            << "Name: '" << std::string(data, size) << "'."
            << std::endl;
    }

    void accept_param_data
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        std::cout
            << "Data: '" << std::string(data, size) << "'."
            << std::endl;
    }

        // Random number in [0, 1].
    double random () {
        return (double(std::rand()) / double(RAND_MAX));
    }

        // Random number in [lower, upper].
    std::size_t random ( std::size_t lower, std::size_t upper )
    {
        const double alpha = random();
        const double blend = double(lower) + alpha*(double(upper-lower));
        return (std::size_t(blend + 0.5));
    }

        // Feed [data, data+size) to parser in random increments.
    void feed ( ::fcgi_iwire * stream, const char * data, std::size_t size )
    {
        std::srand((unsigned)std::time(0));
        for ( std::size_t i = 0; (i < size); ) {
            i += ::fcgi_iwire_feed(stream, data+i, random(1, size-i));
        }
    }

}

int main ( int, char ** )
{
        // Setup FastCGI record parser.
    ::fcgi_iwire_settings limits;
    ::fcgi_iwire stream;
    ::fcgi_iwire_init(&limits, &stream);
        // Register callbacks.
    stream.accept_record       = &::accept_record;
    stream.finish_record       = &::finish_record;
    stream.accept_request      = &::accept_request;
    stream.cancel_request      = &::cancel_request;
    stream.accept_param_name   = &::accept_param_name;
    stream.accept_param_data   = &::accept_param_data;
    stream.accept_content_stdi = &::accept_content_stdi;
        // Feed multiple requests.
    ::feed(&stream, DATA1, SIZE1);
    ::feed(&stream, DATA2, SIZE2);
    ::feed(&stream, DATA3, SIZE3);
    ::feed(&stream, DATA4, SIZE4);
    ::feed(&stream, DATA3, SIZE3);
}
