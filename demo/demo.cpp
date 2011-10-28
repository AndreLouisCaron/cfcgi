// Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include <fcgi.h>
#include <fcgi.hpp>

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

    void write_stream ( fcgi_owire * stream, const char * data, size_t size )
    {
        for ( size_t i = 0; (i < size); ++i ) {
            std::cout << int(data[i]) << ", ";
        }
    }

    void flush_stream ( fcgi_owire * stream )
    {
        std::cout << std::endl;
    }

}

void iwire_test ()
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
    //stream.accept_headers      = &::accept_headers;
    //stream.finish_headers      = &::finish_headers;
    stream.accept_content_stdi = &::accept_content_stdi;
        // Feed multiple requests.
    ::feed(&stream, DATA1, SIZE1);
    ::feed(&stream, DATA2, SIZE2);
    ::feed(&stream, DATA3, SIZE3);
    ::feed(&stream, DATA4, SIZE4);
    ::feed(&stream, DATA3, SIZE3);
}

void owire_test ()
{
      // prepare a fastcgi output stream.
    ::fcgi_owire_settings limits;
    ::fcgi_owire          stream;
    ::fcgi_owire_init(&limits, &stream);
    stream.write_stream = &::write_stream;
    stream.flush_stream = &::flush_stream;
      // sample responder.
    ::fcgi_owire_new_request(&stream, 1, 1);
      // send headers.
    { char data[] = "\013\002SERVER_PORT80";
      ::fcgi_owire_param(&stream, 1, data, sizeof(data)-1);
    }
    { char data[] = "";
      ::fcgi_owire_param(&stream, 1, data, sizeof(data)-1); }
    { char data[] = "";
      ::fcgi_owire_stdi(&stream, 1, data, sizeof(data)-1); }
}

#include <sstream>

std::string ostream_request ()
{
    std::ostringstream buffer;
    fcgi::ostream stream(buffer);
      // start request.
    stream.new_request(1, 1);
      // send request headers.
    stream.param(1, "\013\002SERVER_PORT80");
    stream.param(1);
      // send request body.
    stream.stdi(1, "Hello, application!");
    stream.stdi(1);
      // request complete.
    return (buffer.str());
}

std::string ostream_response ()
try
{
    std::ostringstream buffer;
    fcgi::ostream stream(buffer);
      // send response body.
    stream.stdo(1, "Hello, gateway!");
    stream.stdo(1);
      // response complete.
    stream.end_request(1, EXIT_SUCCESS);
    return (buffer.str());
}
catch ( const std::exception& error )
{
    std::ostringstream buffer;
    fcgi::ostream stream(buffer);
      // send response body.
    stream.stde(1, error.what());
    stream.stde(1);
      // response complete.
    stream.end_request(1, EXIT_FAILURE);
    return (buffer.str());
}

void advanced_test ()
{
    class Test :
        public fcgi::Gateway,
        public fcgi::Application
    {
        /* gateway. */
    protected:
        virtual void gsend ( const std::string& data )
        {
            Application::afeed(data);
        }

        virtual void reply
            ( const std::string& name, const std::string& data )
        {
            std::cout
                << name << "=" << data
                << std::endl;
            if ( name == "FCGI_MAX_CONNS" )
            {
            }
            if ( name == "FCGI_MAX_REQS" )
            {
            }
            if ( name == "FCGI_MPXS_CONNS" )
            {
            }
        }

        virtual void complete ( fcgi::Response& response )
        {
            std::cout
                << "Output='" << response.output() << "'."
                << std::endl
                << "Errors='" << response.errors() << "'."
                << std::endl
                << std::endl;
        }

        /* application. */
    protected:
        virtual void asend ( const std::string& data )
        {
            Gateway::gfeed(data);
        }

        virtual void query
            ( const std::string& name, const std::string& data )
        {
            std::cout
                << name << "=? ('" << data << "')"
                << std::endl;
            if ( name == "FCGI_MAX_CONNS" )
            {
                  // number of children..?
                Application::reply(name, "1");
            }
            if ( name == "FCGI_MAX_REQS" )
            {
                  // don't multiplex.
                Application::reply(name, "1");
            }
            if ( name == "FCGI_MPXS_CONNS" )
            {
                  // don't multiplex.
                Application::reply(name, "0");
            }
        }

        virtual void end_of_head ( fcgi::Request& request )
        {
            const fcgi::Headers& headers = request.head();
            std::cout
                << "Head:"
                << std::endl;
            fcgi::Headers::const_iterator current = headers.begin();
            const fcgi::Headers::const_iterator end = headers.end();
            for ( ; (current != end); ++current )
            {
                std::cout
                    << "  " << current->first << "='" << current->second << "'."
                    << std::endl;
            }
            std::cout
                << std::endl;
        }

        virtual void end_of_body ( fcgi::Request& request )
        {
            std::cout
                << "Body='" << request.body() << "'."
                << std::endl;
            
              // send response.
            Application::output("Hello, FastCGI gateway!");
            Application::output();
            Application::errors("That was easy!");
            Application::errors();
            
              // complete request.
            Application::end_request();
        }
    };
    
      // start the "connection".
    Test test;
    test.Gateway::query("FCGI_MAX_CONNS");
    test.Gateway::query("FCGI_MAX_REQS");
    test.Gateway::query("FCGI_MPXS_CONNS");
    
      // start a request.
    test.Gateway::new_request(1);
    
      // send request headers.
    test.Gateway::head("Authorization", "QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
    test.Gateway::head();
    
      // send request content.
    test.Gateway::body("Hello, FastCGI application!");
    test.Gateway::body();
}

int main ( int, char ** )
{
    advanced_test();
}
