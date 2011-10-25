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

        std::string myPName; std::string myPData;
        std::string myQName; std::string myQData;
        std::string myRName; std::string myRData;

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
            myWire.finish_request      = &istream::finish_request;
            myWire.accept_param_name   = &istream::accept_param_name;
            myWire.accept_param_data   = &istream::accept_param_data;
            myWire.accept_param        = &istream::accept_param;
            myWire.accept_query_name   = &istream::accept_query_name;
            myWire.accept_query_data   = &istream::accept_query_data;
            myWire.accept_query        = &istream::accept_query;
            myWire.accept_reply_name   = &istream::accept_reply_name;
            myWire.accept_reply_data   = &istream::accept_reply_data;
            myWire.accept_reply        = &istream::accept_reply;
            myWire.accept_content_stdi = &istream::accept_content_stdi;
            myWire.accept_content_stdo = &istream::accept_content_stdo;
            myWire.accept_content_stde = &istream::accept_content_stde;
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

    protected:
        virtual void new_request ( int role, int flags )
        {
            switch ( role )
            {
            case 1: // responder.
                break;
            case 2: // authorizer.
                break;
            case 3: // filter.
                break;
            default: {}
            }
        }

        virtual void bad_request ()
        {
        }

        virtual void end_request ()
        {
        }

        virtual void new_record ( int version, int request, int size )
        {
        }

        virtual void end_record ()
        {
        }

        virtual void param_name ( const char * data, size_t size )
        {
            param_name(std::string(data, size));
        }

        virtual void param_name ( const std::string& name )
        {
            myPName = name;
        }

        virtual void param_data ( const char * data, size_t size )
        {
            param_data(std::string(data, size));
        }

        virtual void param_data ( const std::string& data )
        {
            myPData = data;
        }

        virtual void param ()
        {
            param(myPName, myPData), myPName.clear(), myPData.clear();
        }

        virtual void param ( const std::string& name, const std::string& data )
        {
        }

        virtual void query_name ( const char * data, size_t size )
        {
            query_name(std::string(data, size));
        }

        virtual void query_name ( const std::string& name )
        {
            myQName = name;
        }

        virtual void query_data ( const char * data, size_t size )
        {
            query_data(std::string(data, size));
        }

        virtual void query_data ( const std::string& data )
        {
            myQData = data;
        }

        virtual void query ()
        {
            query(myQName, myQData), myQName.clear(), myQData.clear();
        }

        virtual void query ( const std::string& name, const std::string& data )
        {
        }

        virtual void reply_name ( const char * data, size_t size )
        {
            reply_name(std::string(data, size));
        }

        virtual void reply_name ( const std::string& name )
        {
            myRName = name;
        }

        virtual void reply_data ( const char * data, size_t size )
        {
            reply_data(std::string(data, size));
        }

        virtual void reply_data ( const std::string& data )
        {
            myRData = data;
        }

        virtual void reply ()
        {
            reply(myRName, myRData), myRName.clear(), myRData.clear();
        }

        virtual void reply ( const std::string& name, const std::string& data )
        {
        }

        virtual void stdi ( const char * data, size_t size )
        {
            stdi(std::string(data, size));
        }
        virtual void stdi ( const std::string& ) {}

        virtual void stdo ( const char * data, size_t size )
        {
            stdo(std::string(data, size));
        }
        virtual void stdo ( const std::string& ) {}

        virtual void stde ( const char * data, size_t size )
        {
            stde(std::string(data, size));
        }
        virtual void stde ( const std::string& ) {}

        /* class methods. */
    private:
        static void accept_record
            ( ::fcgi_iwire * stream, int version, int request, int content )
        {
            static_cast<istream*>(stream->object)
                ->new_record(version, request, content);
        }

        static void finish_record ( ::fcgi_iwire * stream )
        {
            static_cast<istream*>(stream->object)->end_record();
        }

        static void accept_request ( ::fcgi_iwire * stream, int role, int flags )
        {
            static_cast<istream*>(stream->object)->new_request(role, flags);
        }

        static void cancel_request ( ::fcgi_iwire * stream )
        {
            static_cast<istream*>(stream->object)->bad_request();
        }

        static void finish_request ( ::fcgi_iwire * stream, uint32_t, uint8_t )
        {
            static_cast<istream*>(stream->object)->end_request();
        }

        static void accept_content_stdi
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->stdi(data, size);
        }

        static void accept_content_stdo
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->stdo(data, size);
        }

        static void accept_content_stde
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->stde(data, size);
        }

        static void accept_param_name
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->param_name(data, size);
        }

        static void accept_param_data
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->param_data(data, size);
        }

        static void accept_param ( ::fcgi_iwire * stream )
        {
            static_cast<istream*>(stream->object)->param();
        }

        static void accept_query_name
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->query_name(data, size);
        }

        static void accept_query_data
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->query_data(data, size);
        }

        static void accept_query ( ::fcgi_iwire * stream )
        {
            static_cast<istream*>(stream->object)->query();
        }

        static void accept_reply_name
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->reply_name(data, size);
        }

        static void accept_reply_data
            ( ::fcgi_iwire * stream, const char * data, size_t size )
        {
            static_cast<istream*>(stream->object)->reply_data(data, size);
        }

        static void accept_reply ( ::fcgi_iwire * stream )
        {
            static_cast<istream*>(stream->object)->reply();
        }
    };

}

#endif /* _fcgi_istream_hpp__ */
