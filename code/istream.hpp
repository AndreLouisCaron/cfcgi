#ifndef _fcgi_istream_hpp__
#define _fcgi_istream_hpp__

// Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "fcgi.h"
#include <iostream>
#include <string>

namespace fcgi {

    class istream
    {
        /* data. */
    private:
        ::fcgi_iwire_settings mySettings;
        ::fcgi_iwire myWire;

        /* construction. */
    public:
        istream ()
        {
            ::fcgi_iwire_init(&mySettings, &myWire);
            myWire.object = static_cast<void*>(this);
              // Register callbacks.
            myWire.accept_record       = &istream::accept_record;
            myWire.finish_record       = &istream::finish_record;
            myWire.accept_request      = &istream::accept_request;
            myWire.cancel_request      = &istream::cancel_request;
            myWire.accept_param_name   = &istream::accept_param_name;
            myWire.accept_param_data   = &istream::accept_param_data;
            myWire.accept_content_stdi = &istream::accept_content_stdi;
        }

        /* methods. */
    public:
        void feed ( const char * data, size_t size )
        {
            ::fcgi_iwire_feed(&myWire, data, size);
        }

        void feed ( const std::string& buffer )
        {
            ::fcgi_iwire_feed(&myWire, buffer.data(), buffer.size());
        }

        /* class methods. */
    private:
        static void accept_record
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

        static void finish_record ( ::fcgi_iwire * stream )
        {
            std::cerr
                << "Finish record."
                << std::endl;
        }

        static void accept_request ( ::fcgi_iwire * stream, int role, int flags )
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

        static void cancel_request ( ::fcgi_iwire * stream )
        {
            std::cerr
                << "Cancel request."
                << std::endl;
        }

        static void accept_content_stdi
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            std::cout
                << "stdin: '" << std::string(data, size) << "'."
                << std::endl;
        }

        static void accept_param_name
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            std::cout
                << "Name: '" << std::string(data, size) << "'."
                << std::endl;
        }

        static void accept_param_data
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            std::cout
                << "Data: '" << std::string(data, size) << "'."
                << std::endl;
        }
    };

}

#endif /* _fcgi_istream_hpp__ */
