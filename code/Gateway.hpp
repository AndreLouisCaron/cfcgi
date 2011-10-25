#ifndef _fcgi_Gateway_hpp__
#define _fcgi_Gateway_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "fcgi.h"
#include "Response.hpp"

#include <map>

namespace fcgi {

    /*!
     * @brief High-level interface for implementing a FastCGI gateway.
     */
    class Gateway
    {
        /* nested types. */
    private:
        typedef std::map<Response::Id, Response> Responses;
        typedef Responses::value_type Mapping;
        typedef Responses::iterator Selection;

        /* data. */
    private:
        Responses myResponses;
        Selection mySelection;

          // buffer for reply to query.
        std::string myRName;
        std::string myRData;

        ::fcgi_iwire_settings myISettings; ::fcgi_iwire myIWire;
        ::fcgi_owire_settings myOSettings; ::fcgi_owire myOWire;

        /* construction. */
    public:
        Gateway ();

        /* methods. */
    public:
        /*!
         * @brief Process new record(s) received from peer (the application).
         */
        void gfeed ( const char * data, size_t size );

        /*!
         * @brief Process new record(s) received from peer (the application).
         */
        void gfeed ( const std::string& buffer );

        void query ( const std::string& name );

        void new_request ( uint16_t request );

        void head ( const std::string& name, const std::string& data );
        void head ();
        void body ( const std::string& name );
        void body ();

    protected:
        virtual void gsend ( const char * data, size_t size )
        {
            gsend(std::string(data, size));
        }

        virtual void gsend ( const std::string& data ) {}

        /*!
         * @brief Notification that additional output content is available.
         */
        virtual void output ( Response& response ) {}

        /*!
         * @brief Notification that additional error content is available.
         */
        virtual void errors ( Response& response ) {}

        /*!
         * @brief Notification that all the output was received.
         */
        virtual void end_of_output ( Response& response ) {}

        /*!
         * @brief Notification that all the errors were received.
         */
        virtual void end_of_errors ( Response& response ) {}

        /*!
         * @brief Notification that the application finished processing.
         */
        virtual void complete ( Response& response ) = 0;

        /*!
         * @brief Notification the reply to a query has arrived.
         */
        virtual void reply
            ( const std::string& name, const std::string& data ) = 0;

        /* class methods. */
    private:
        static void accept_record
            ( ::fcgi_iwire * stream, int version, int request, int content );
        static void finish_record ( ::fcgi_iwire * stream );

        static void accept_reply_name
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void accept_reply_data
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void accept_reply ( ::fcgi_iwire * stream );

        static void accept_content_stdo
            ( ::fcgi_iwire * stream, const char * data, size_t size );
        static void accept_content_stde
            ( ::fcgi_iwire * stream, const char * data, size_t size );

        static void finish_request ( ::fcgi_iwire * stream, uint32_t, uint8_t );

        static void write_stream
            ( ::fcgi_owire * stream, const char * data, size_t size );
    };

}

#endif /* _fcgi_Gateway_hpp__ */
