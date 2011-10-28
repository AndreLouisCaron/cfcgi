#ifndef _fcgi_Headers_hpp__
#define _fcgi_Headers_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Headers.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

#include "fcgi.h"
#include <map>
#include <string>

namespace fcgi {

    /*!
     * @group application
     * @brief Convenient storage for HTTP request headers.
     */
    class Headers
    {
        /* nested types. */
    public:
        typedef std::map<std::string, std::string> Mapping;
        typedef Mapping::const_iterator const_iterator;

        /* data. */
    private:
        Mapping myMapping;

        ::fcgi_ipstream myPStream;

        std::string myName;
        std::string myData;

        /* construction. */
    public:
        Headers ();
        Headers ( const Headers& other );

        /* methods. */
    public:
        void feed ( const char * data, size_t size );
        void feed ( const std::string& content );

        std::string get ( const std::string& name ) const;
        std::string get
            ( const std::string& name, const std::string& fallback ) const;

        const_iterator begin () const;
        const_iterator end () const;

        void clear ();

        /* class methods. */
    private:
        static void accept
            ( ::fcgi_ipstream * stream, size_t nsize, size_t dsize );
        static void accept_name
            ( ::fcgi_ipstream * stream, const char * data, size_t size );
        static void accept_data
            ( ::fcgi_ipstream * stream, const char * data, size_t size );

        static void finish_name ( ::fcgi_ipstream * stream );
        static void finish_data ( ::fcgi_ipstream * stream );
        static void finish ( ::fcgi_ipstream * stream );
    };

}

#endif /* _fcgi_Headers_hpp__ */
