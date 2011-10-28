// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Headers.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

#include "Headers.hpp"
#include <iostream>

namespace fcgi {

    Headers::Headers ()
    {
        ::fcgi_ipstream_init(&myPStream);
        myPStream.object      = this;
        myPStream.accept      = &Headers::accept;
        myPStream.accept_name = &Headers::accept_name;
        myPStream.finish_name = &Headers::finish_name;
        myPStream.accept_data = &Headers::accept_data;
        myPStream.finish_data = &Headers::finish_data;
        myPStream.finish      = &Headers::finish;
    }

    Headers::Headers ( const Headers& other )
        : myMapping(other.myMapping),
          myName(other.myName),
          myData(other.myData)
    {
        ::fcgi_ipstream_init(&myPStream);
        myPStream.object      = this;
        myPStream.accept      = &Headers::accept;
        myPStream.accept_name = &Headers::accept_name;
        myPStream.finish_name = &Headers::finish_name;
        myPStream.accept_data = &Headers::accept_data;
        myPStream.finish_data = &Headers::finish_data;
        myPStream.finish      = &Headers::finish;
    }

    void Headers::feed ( const char * data, size_t size )
    {
        ::fcgi_ipstream_feed(&myPStream, data, size);
    }

    void Headers::feed ( const std::string& content )
    {
        feed(content.data(), content.size());
    }

    std::string Headers::get ( const std::string& name ) const
    {
        return (get(name, std::string()));
    }

    std::string Headers::get
        ( const std::string& name, const std::string& fallback ) const
    {
        Mapping::const_iterator match = myMapping.find(name);
        if ( match == myMapping.end() ) {
            return (fallback);
        }
        return (match->second);
    }

    Headers::const_iterator Headers::begin () const
    {
        return (myMapping.begin());
    }

    Headers::const_iterator Headers::end () const
    {
        return (myMapping.end());
    }

    void Headers::clear ()
    {
          // Clear contents, re-use buffers.
        Mapping::iterator current = myMapping.begin();
        const Mapping::iterator end = myMapping.end();
        for ( ; (current != end); ++current ) {
            current->second.clear();
        }
    }

    void Headers::accept
            ( ::fcgi_ipstream * stream, size_t nsize, size_t dsize )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
        headers.myName.reserve(nsize);
        headers.myData.reserve(dsize);
    }

    void Headers::accept_name
        ( ::fcgi_ipstream * stream, const char * data, size_t size )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
        headers.myName.append(data, size);
    }

    void Headers::accept_data
        ( ::fcgi_ipstream * stream, const char * data, size_t size )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
        headers.myData.append(data, size);
    }

    void Headers::finish_name ( ::fcgi_ipstream * stream )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
        if ( headers.myName.size() != stream->nsize )
        {
            std::cerr
                << "[fcgi::Headers] Invalid name size!"
                << std::endl;
        }
    }

    void Headers::finish_data ( ::fcgi_ipstream * stream )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
        if ( headers.myData.size() != stream->dsize )
        {
            std::cerr
                << "[fcgi::Headers] Invalid data size!"
                << std::endl;
        }
    }

    void Headers::finish ( ::fcgi_ipstream * stream )
    {
        Headers& headers = *static_cast<Headers*>(stream->object);
          // commit header.
        headers.myMapping.insert(
            std::make_pair(headers.myName, headers.myData));
          // clear contents, re-use buffers.
        headers.myName.clear();
        headers.myData.clear();
    }

}
