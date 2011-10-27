#ifndef _fcgi_Application_hpp__
#define _fcgi_Application_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "fcgi.h"
#include "Request.hpp"

#include <map>

namespace fcgi {

    class Application
    {
        /* nested types. */
    private:
        typedef std::map<Request::Id, Request> Requests;
        typedef Requests::value_type Mapping;
        typedef Requests::iterator Selection;

        /* data. */
    private:
        Requests myRequests;
        Selection mySelection;

        ::fcgi_iwire_settings myISettings; ::fcgi_iwire myIWire;
        ::fcgi_owire_settings myOSettings; ::fcgi_owire myOWire;

          // buffer for query.
        std::string myQName;
        std::string myQData;

          // buffer for param.
        std::string myPName;
        std::string myPData;

        /* construction. */
    public:
        Application ();

        /* methods. */
    public:
        /*!
         * @brief Process new record(s) received from peer (the application).
         */
        void afeed ( const char * data, size_t size );

        /*!
         * @brief Process new record(s) received from peer (the application).
         */
        void afeed ( const std::string& buffer );

        void reply ( const std::string& name, const std::string& data );

        void output ( const std::string& output );
        void output ();
        void errors ( const std::string& output );
        void errors ();

        void end_request ( uint32_t astatus=0, uint8_t pstatus=0 );

    protected:
        virtual void asend ( const char * data, size_t size )
        {
            asend(std::string(data, size));
        }

        virtual void asend ( const std::string& data ) {}

         /*!
         * @brief Notification a query has arrived.
         */
        virtual void query
            ( const std::string& name, const std::string& data ) = 0;

        /*!
         * @brief Notification that all the headers were received.
         */
        virtual void end_of_head ( Request& request ) = 0;

        /*!
         * @brief Notification that additional body content is available.
         */
        virtual void body ( Request& request ) {}

        /*!
         * @brief Notification that all the body content was received.
         */
        virtual void end_of_body ( Request& request ) = 0;

        /* class methods. */
    private:
        static void accept_record
            ( ::fcgi_iwire * stream, int version, int request, int content );
        static void finish_record ( ::fcgi_iwire * stream );

        static void accept_query_name
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void accept_query_data
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void accept_query ( ::fcgi_iwire * stream );

        static void accept_request
            ( ::fcgi_iwire * stream, int role, int flags );

        static void accept_headers
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void finish_headers ( ::fcgi_iwire * stream );

        static void accept_content_stdi
            ( ::fcgi_iwire * stream, const char * data, size_t size );

        static void write_stream
            ( ::fcgi_owire * stream, const char * data, size_t size );
    };

}

#endif /* _fcgi_Application_hpp__ */
