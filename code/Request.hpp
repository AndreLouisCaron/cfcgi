#ifndef _fcgi_Request_hpp__
#define _fcgi_Request_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "fcgi.h"
#include <map>
#include <string>

namespace fcgi {

    /*!
     * @brief FastCGI request from the application's point of view.
     */
    class Request
    {
        /* nested types. */
    public:
        typedef uint16_t Id;

        typedef std::map<std::string, std::string> Head;

        /* data. */
    private:
        const Id myId;
        std::string myHead;
        std::string myBody;

        bool myPrepared;
        bool myComplete;

        /* construction. */
    public:
        Request ( Id id )
            : myId(id)
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

        Head head () const
        {
            return (Head());
        }

        std::string& body ()
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
