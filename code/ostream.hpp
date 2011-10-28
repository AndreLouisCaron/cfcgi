#ifndef _fcgi_ostream_hpp__
#define _fcgi_ostream_hpp__

// Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file ostream.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief FastCGI wire protocol testing utility.
 */

#include "fcgi.h"
#include <ostream>

namespace fcgi {

    class ostream
    {
        /* data. */
    private:
        std::ostream& myStream;
        ::fcgi_owire_settings mySettings;
        ::fcgi_owire myWire;

        /* construction. */
    public:
        ostream ( std::ostream& stream )
            : myStream(stream)
        {
            ::fcgi_owire_init(&mySettings, &myWire);
            myWire.object = static_cast<void*>(this);
              // Register callbacks.
            myWire.write_stream = &ostream::write_stream;
            myWire.flush_stream = &ostream::flush_stream;
        }

        /* methods. */
    public:
        void new_request ( uint16_t request, int role )
        {
            ::fcgi_owire_new_request(&myWire, request, role);
        }

        void bad_request ( uint16_t request )
        {
            ::fcgi_owire_bad_request(&myWire, request);
        }

        void end_request
            ( uint16_t request, uint32_t astatus=0, uint8_t pstatus=0 )
        {
            ::fcgi_owire_end_request(&myWire, request, astatus, pstatus);
        }

        void cant_multiplex ( uint16_t request, uint32_t astatus=0 )
        {
            end_request(request, astatus, 1);
        }

        void overloaded ( uint16_t request, uint32_t astatus=0 )
        {
            end_request(request, astatus, 2);
        }

        void unknown_role ( uint16_t request, uint32_t astatus=0 )
        {
            end_request(request, astatus, 3);
        }

        void param ( uint16_t request, const char * data, uint16_t size )
        {
            ::fcgi_owire_param(&myWire, request, data, size);
        }

        void param ( uint16_t request, const std::string& record )
        {
            param(request, record.data(), record.size());
        }

        void param ( uint16_t request )
        {
            param(request, 0, 0);
        }

        void stdi ( uint16_t request, const char * data, size_t size )
        {
            ::fcgi_owire_stdi(&myWire, request, data, size);
        }

        void stdi ( uint16_t request, const std::string& record )
        {
            stdi(request, record.data(), record.size());
        }

        void stdi ( uint16_t request )
        {
            stdi(request, 0, 0);
        }

        void stdo ( uint16_t request, const char * data, size_t size )
        {
            ::fcgi_owire_stdo(&myWire, request, data, size);
        }

        void stdo ( uint16_t request, const std::string& record )
        {
            stdo(request, record.data(), record.size());
        }

        void stdo ( uint16_t request )
        {
            stdo(request, 0, 0);
        }

        void stde ( uint16_t request, const char * data, size_t size )
        {
            ::fcgi_owire_stde(&myWire, request, data, size);
        }

        void stde ( uint16_t request, const std::string& record )
        {
            stde(request, record.data(), record.size());
        }

        void stde ( uint16_t request )
        {
            stde(request, 0, 0);
        }

        void extra ( uint16_t request, const char * data, size_t size )
        {
            ::fcgi_owire_extra(&myWire, request, data, size);
        }

        void extra ( uint16_t request, const std::string& record )
        {
            extra(request, record.data(), record.size());
        }

        void extra ( uint16_t request )
        {
            extra(request, 0, 0);
        }

        void query ( const char * data, uint16_t size )
        {
            ::fcgi_owire_query(&myWire, data, size);
        }

        void query ( const std::string& record )
        {
            query(record.data(), record.size());
        }

        void reply ( const char * data, uint16_t size )
        {
            ::fcgi_owire_query(&myWire, data, size);
        }

        void reply ( const std::string& record )
        {
            reply(record.data(), record.size());
        }

        /* class methods. */
    private:
        static void write_stream
            ( fcgi_owire * stream, const char * data, size_t size )
        {
            static_cast<ostream*>(stream->object)->myStream.write(data, size);
        }

        static void flush_stream ( fcgi_owire * stream )
        {
            static_cast<ostream*>(stream->object)->myStream.flush();
        }
    };

}

#endif /* _fcgi_ostream_hpp__ */
