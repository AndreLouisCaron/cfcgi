#ifndef _fcgi_Role_hpp__
#define _fcgi_Role_hpp__

// Copyright(c) 2011, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file Role.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

namespace fcgi {

    class Role
    {
	/* nested types. */
    public:
	typedef uint16_t Value;

	/* data. */
    private:
	Value myValue;

	/* construction. */
    private:
	Role ( Value value )
	    : myValue(value)
	{}

    public:
	Role ()
	    : myValue(0)
	{}

	/* methods. */
    public:
	static const Role unknown ()
	{
	    return (0);
	}

	static const Role responder ()
	{
	    return (1);
	}

	static const Role authorizer ()
	{
	    return (2);
	}

	static const Role filter ()
	{
	    return (3);
	}

	/* operators. */
    public:
	bool operator== ( const Role& rhs ) const
	{
	    return (myValue == rhs.myValue);
	}

	bool operator!= ( const Role& rhs ) const
	{
	    return (myValue != rhs.myValue);
	}
    };

}

#endif /* _fcgi_Role_hpp__ */
