#ifndef _fcgi_Authorizer_hpp__
#define _fcgi_Authorizer_hpp__

// Copyright(c) 2011-2012, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Authorizer.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

#include "Application.hpp"
#include <sstream>

namespace fcgi {

    class Authorizer :
        public Application
    {
        /* contract. */
    protected:
        /*!
         * @brief Handle authorization request.
         */
        virtual void handle_authorization (fcgi::Request& request) = 0;

        /* overrides. */
    protected:
        virtual void end_of_head (fcgi::Request& request)
        {
            // Only handle authorization requests.
            if (request.role() == fcgi::Role::authorizer()) {
                handle_authorization(request);
            }
            else {
                errors(
                    "This is an authorizer, not a responder or filter."
                );
                errors();
                output();
                end_request(1);
            }
        }

        virtual void body (fcgi::Request& request) {}
        virtual void end_of_body (fcgi::Request& request) {}
    };

}

#endif /* _fcgi_Authorizer_hpp__ */
