#ifndef _fcgi_Response_hpp__
#define _fcgi_Response_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "fcgi.h"
#include <string>

namespace fcgi {

    /*!
     * @brief FastCGI response from the gateway's point of view.
     */
    class Response
    {
        /* nested types. */
    public:
        typedef uint16_t Id;

        /* data. */
    private:
        const Id myId;

        std::string myOutput;
        std::string myErrors;
        uint32_t    myStatus;
        bool      myComplete;

        /* construction. */
    public:
        explicit Response ( Id id )
            : myId(id), myStatus(0), myComplete(false)
        {}

        /* methods. */
    public:
        Id id () const
        {
            return (myId);
        }

        void clear ()
        {
            myOutput.clear();
            myErrors.clear();
            myStatus = 0;
            myComplete = false;
        }

        std::string& output ()
        {
            return (myOutput);
        }

        std::string& errors ()
        {
            return (myErrors);
        }

        uint32_t status () const
        {
            return (myStatus);
        }

        void status ( uint32_t status )
        {
            myStatus = status;
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

#endif /* _fcgi_Response_hpp__ */
