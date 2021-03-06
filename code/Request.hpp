#ifndef _fcgi_Request_hpp__
#define _fcgi_Request_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Request.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

#include "fcgi.h"
#include "Headers.hpp"
#include "Role.hpp"

namespace fcgi {

    /*!
     * @group application
     * @brief Convenient storage for an HTTP request from the gateway.
     */
    class Request
    {
        /* nested types. */
    public:
        typedef uint16_t Id;

        /* data. */
    private:
        const Id myId;
	Role myRole;
        Headers myHead;
        std::string myBody;

        bool myPrepared;
        bool myComplete;

        /* construction. */
    public:
        Request ( Id id )
            : myId(id), myHead()
        {}

        /* methods. */
    public:
        Id id () const
        {
            return (myId);
        }

        void clear ()
        {
            myHead.clear();
            myBody.clear();
        }

	void role ( const Role& role )
	{
	    myRole = role;
	}

	const Role& role () const
	{
	    return (myRole);
	}

        Headers& head ()
        {
            return (myHead);
        }

        const Headers& head () const
        {
            return (myHead);
        }

        std::string& body ()
        {
            return (myBody);
        }

        const std::string& body () const
        {
            return (myBody);
        }

        bool prepared () const
        {
            return (myPrepared);
        }

        void prepared ( bool prepared )
        {
            myPrepared = prepared;
        }

        bool complete () const
        {
            return (myComplete);
        }

        void complete ( bool complete )
        {
            myComplete = complete;
        }
    };

}

#endif /* _fcgi_Request_hpp__ */
