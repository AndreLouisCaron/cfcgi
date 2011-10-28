// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Gateway.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI gateway implementation.
 */

#include "Gateway.hpp"
#include <sstream>
#include <stdexcept>

namespace fcgi {

    Gateway::Gateway ()
        : myResponses(), mySelection(myResponses.end())
    {
        ::fcgi_iwire_init(&myISettings, &myIWire);
        myIWire.object = static_cast<void*>(this);
          // Register callbacks.
        myIWire.accept_record       = &Gateway::accept_record;
        myIWire.finish_record       = &Gateway::finish_record;
        myIWire.finish_request      = &Gateway::finish_request;
        myIWire.accept_reply_name   = &Gateway::accept_reply_name;
        myIWire.accept_reply_data   = &Gateway::accept_reply_data;
        myIWire.accept_reply        = &Gateway::accept_reply;
        myIWire.accept_content_stdo = &Gateway::accept_content_stdo;
        myIWire.accept_content_stde = &Gateway::accept_content_stde;
        
        ::fcgi_owire_init(&myOSettings, &myOWire);
        myOWire.object = static_cast<void*>(this);
          // Register callbacks.
        myOWire.write_stream = &Gateway::write_stream;
    }

    void Gateway::gfeed ( const char * data, size_t size )
    {
        ::fcgi_iwire_feed(&myIWire, data, size);
    }

    void Gateway::gfeed ( const std::string& buffer )
    {
        ::fcgi_iwire_feed(&myIWire, buffer.data(), buffer.size());
    }

    void Gateway::query ( const std::string& name )
    {
          // build the reply (name,value) pair.
        std::ostringstream stream;
        stream
            << (char)name.size() << (char)0 << name << "";
        const std::string query = stream.str();
          // send it.
        ::fcgi_owire_query(&myOWire, query.data(), query.size());
    }

    void Gateway::new_request ( uint16_t request )
    {
        ::fcgi_owire_new_request(&myOWire, request, 1);
          // lookup the response object for the request ID.
        mySelection = myResponses.find(request);
          // might be the first use of this request ID.
        if ( mySelection == myResponses.end() )
        {
            mySelection = myResponses.insert
                (Mapping(request, Response(request))).first;
        }
    }

    void Gateway::set_request ( uint16_t request )
    {
        mySelection = myResponses.find(request);
        if ( mySelection == myResponses.end() ) {
            throw (std::invalid_argument("unused request id."));
        }
    }

    void Gateway::head ( const std::string& name, const std::string& data )
    {
          // locate the request.
        if ( mySelection == myResponses.end() ) {
            return;
        }
        Response& response = mySelection->second;
          // build the reply (name,value) pair.
        std::ostringstream stream;
        stream
            << (char)name.size() << (char)data.size() << name << data;
        const std::string param = stream.str();
          // send it.
        ::fcgi_owire_param(&myOWire, response.id(), param.data(), param.size());
    }

    void Gateway::head ()
    {
          // locate the request.
        if ( mySelection == myResponses.end() ) {
            return;
        }
        Response& response = mySelection->second;
          // send empty record to notify of "end of stream".
        ::fcgi_owire_param(&myOWire, response.id(), 0, 0);
    }

    void Gateway::body ( const std::string& body )
    {
          // locate the request.
        if ( mySelection == myResponses.end() ) {
            return;
        }
        Response& response = mySelection->second;
          // send empty record to notify of "end of stream".
        ::fcgi_owire_stdi(&myOWire, response.id(), body.data(), body.size());
    }

    void Gateway::body ()
    {
          // locate the request.
        if ( mySelection == myResponses.end() ) {
            return;
        }
        Response& response = mySelection->second;
          // send empty record to notify of "end of stream".
        ::fcgi_owire_stdi(&myOWire, response.id(), 0, 0);
    }

    void Gateway::accept_record
        ( ::fcgi_iwire * stream, int version, int request, int content )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
        if ( version != 1 )
        {
            // ...
        }
          // don't create a request object for management records.
        if ( request == 0 ) {
            return;
        }
          // lookup the response object for the request ID.
        gateway.mySelection = gateway.myResponses.find(request);
          // might be the first use of this request ID.
        if ( gateway.mySelection == gateway.myResponses.end() )
        {
            gateway.mySelection = gateway.myResponses.insert
                (Mapping(request, Response(request))).first;
        }
        // TODO: forward content length.
    }

    void Gateway::finish_record ( ::fcgi_iwire * stream )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // ignore invalid records.
        if ( gateway.mySelection == gateway.myResponses.end() ) {
            return;
        }
          // clear selection.
        gateway.mySelection = gateway.myResponses.end();
    }

    void Gateway::finish_request ( ::fcgi_iwire * stream, uint32_t, uint8_t )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // ignore invalid records.
        if ( gateway.mySelection == gateway.myResponses.end() ) {
            return;
        }
        Response& response = gateway.mySelection->second;
          // process end of request.
        gateway.complete(response);
          // clear contents, but keep the buffers.
        response.clear();
    }

    void Gateway::accept_content_stdo
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // ignore invalid records.
        if ( gateway.mySelection == gateway.myResponses.end() ) {
            return;
        }
        // TODO: make sure partial record does not produce {size=0}.
          // accept stream contents.
        Response& response = gateway.mySelection->second;
        if ( size == 0 ) {
            gateway.end_of_output(response);
        }
        else {
            response.output().append(data, size);
            gateway.output(response);
        }
    }

    void Gateway::accept_content_stde
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // ignore invalid records.
        if ( gateway.mySelection == gateway.myResponses.end() ) {
            return;
        }
        // TODO: make sure partial record does not produce {size=0}.
          // accept stream contents.
        Response& response = gateway.mySelection->second;
        if ( size == 0 ) {
            gateway.end_of_errors(response);
        }
        else {
            response.errors().append(data, size);
            gateway.errors(response);
        }
    }

    void Gateway::accept_reply_name
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // accumulate into buffer, commit later.
        gateway.myRName.append(data, size);
    }

    void Gateway::accept_reply_data
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // accumulate into buffer, commit later.
        gateway.myRData.append(data, size);
    }

    void Gateway::accept_reply ( ::fcgi_iwire * stream )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
          // forward buffered response.
        gateway.reply(gateway.myRName, gateway.myRData);
          // clear contents, but keep the buffers.
        gateway.myRName.clear();
        gateway.myRData.clear();
    }

    void Gateway::write_stream
        ( ::fcgi_owire * stream, const char * data, size_t size )
    {
        Gateway& gateway = *static_cast<Gateway*>(stream->object);
        gateway.gsend(data, size);
    }

}
