#ifndef _fcgi_hpp__
#define _fcgi_hpp__

// Copyright(c) Andre Caron (andre.l.caron@gmail.com), 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file fcgi.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Complete API for development of FastCGI gateways and applications.
 */

#include "fcgi.h"

  /*!
   * @brief C++ interface to FastCGI wire protocol.
   */
namespace fcgi {}

#include "ostream.hpp"

#include "Application.hpp"
#include "Gateway.hpp"
#include "Headers.hpp"
#include "Request.hpp"
#include "Response.hpp"

// Application models.
#include "Authorizer.hpp"
#include "HttpBasicAuthorizer.hpp"

#endif /* _fcgi_hpp__ */
