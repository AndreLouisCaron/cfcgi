// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "Application.hpp"
#include <sstream>

namespace fcgi {

    Application::Application ()
        : myRequests(), mySelection(myRequests.end())
    {
        ::fcgi_iwire_init(&myISettings, &myIWire);
        myIWire.object = static_cast<void*>(this);
          // Register callbacks.
        myIWire.accept_record       = &Application::accept_record;
        myIWire.finish_record       = &Application::finish_record;
        myIWire.accept_query_name   = &Application::accept_query_name;
        myIWire.accept_query_data   = &Application::accept_query_data;
        myIWire.accept_query        = &Application::accept_query;
        myIWire.accept_request      = &Application::accept_request;
        myIWire.accept_headers      = &Application::accept_headers;
        myIWire.finish_headers      = &Application::finish_headers;
        myIWire.accept_content_stdi = &Application::accept_content_stdi;
        
        ::fcgi_owire_init(&myOSettings, &myOWire);
        myOWire.object = static_cast<void*>(this);
          // Register callbacks.
        myOWire.write_stream = &Application::write_stream;
    }

    void Application::afeed ( const char * data, size_t size )
    {
        ::fcgi_iwire_feed(&myIWire, data, size);
    }

    void Application::afeed ( const std::string& buffer )
    {
        ::fcgi_iwire_feed(&myIWire, buffer.data(), buffer.size());
    }

    void Application::reply ( const std::string& name, const std::string& data )
    {
          // build the reply (name,value) pair.
        std::ostringstream stream;
        stream
            << (char)name.size() << (char)data.size() << name << data;
        const std::string reply = stream.str();
          // send it.
        ::fcgi_owire_reply(&myOWire, reply.data(), reply.size());
    }

    void Application::output ( const std::string& output )
    {
          // ignore invalid records.
        if ( mySelection == myRequests.end() ) {
            return;
        }
        Request& request = mySelection->second;
        ::fcgi_owire_stdo(&myOWire, request.id(), output.data(), output.size());
    }

    void Application::output ()
    {
        if ( mySelection == myRequests.end() ) {
            return;
        }
        Request& request = mySelection->second;
        ::fcgi_owire_stdo(&myOWire, request.id(), 0, 0);
    }

    void Application::errors ( const std::string& errors )
    {
        if ( mySelection == myRequests.end() ) {
            return;
        }
        Request& request = mySelection->second;
        ::fcgi_owire_stde(&myOWire, request.id(), errors.data(), errors.size());
    }

    void Application::errors ()
    {
        if ( mySelection == myRequests.end() ) {
            return;
        }
        Request& request = mySelection->second;
        ::fcgi_owire_stde(&myOWire, request.id(), 0, 0);
    }

    void Application::end_request ( uint32_t astatus, uint8_t pstatus )
    {
        if ( mySelection == myRequests.end() ) {
            return;
        }
        Request& request = mySelection->second;
        ::fcgi_owire_end_request(&myOWire, request.id(), astatus, pstatus);
          // clear contents, but keep buffers.
        request.clear();
          // invalidate selection.
        mySelection = myRequests.end();
    }

    void Application::accept_record
        ( ::fcgi_iwire * stream, int version, int request, int content )
    {
        Application& application = *static_cast<Application*>(stream->object);
        if ( version != 1 )
        {
            // ...
        }
          // don't create a request object for management records.
        if ( request == 0 ) {
            return;
        }
          // lookup the request object for the request ID.
        application.mySelection = application.myRequests.find(request);
          // might be the first use of this request ID.
        if ( application.mySelection == application.myRequests.end() )
        {
            application.mySelection = application.myRequests.insert
                (Mapping(request, Request(request))).first;
        }
        // TODO: forward content length.
    }

    void Application::finish_record ( ::fcgi_iwire * stream )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // ignore invalid records.
        if ( application.mySelection == application.myRequests.end() ) {
            return;
        }
          // clear selection.
        application.mySelection = application.myRequests.end();
    }

    void Application::accept_query_name
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // accumulate into buffer, commit later.
        application.myQName.append(data, size);
    }

    void Application::accept_query_data
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // accumulate into buffer, commit later.
        application.myQData.append(data, size);
    }

    void Application::accept_query ( ::fcgi_iwire * stream )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // forward buffered request.
        application.query(application.myQName, application.myQData);
          // clear contents, but keep the buffers.
        application.myQName.clear();
        application.myQData.clear();
    }

    void Application::accept_request
        ( ::fcgi_iwire * stream, int role, int flags )
    {
    }

    void Application::accept_headers
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // ignore invalid records.
        if ( application.mySelection == application.myRequests.end() ) {
            return;
        }
        Request& request = application.mySelection->second;
        request.head().feed(data, size);
    }

    void Application::finish_headers ( ::fcgi_iwire * stream )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // ignore invalid records.
        if ( application.mySelection == application.myRequests.end() ) {
            return;
        }
        Request& request = application.mySelection->second;
        request.prepared(true);
        application.end_of_head(request);
    }

    void Application::accept_content_stdi
        ( ::fcgi_iwire * stream, const char * data, size_t size )
    {
        Application& application = *static_cast<Application*>(stream->object);
          // ignore invalid records.
        if ( application.mySelection == application.myRequests.end() ) {
            return;
        }
        // TODO: make sure partial record does not produce {size=0}.
          // accept stream contents.
        Request& request = application.mySelection->second;
        if ( size == 0 ) {
            application.end_of_body(request);
        }
        else {
            request.body().append(data, size);
            application.body(request);
        }
    }

    void Application::write_stream
        ( ::fcgi_owire * stream, const char * data, size_t size )
    {
        Application& application = *static_cast<Application*>(stream->object);
        application.asend(data, size);
    }

}
