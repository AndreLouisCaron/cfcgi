#ifndef _fcgi_HttpBasicAuthorizer_hpp__
#define _fcgi_HttpBasicAuthorizer_hpp__

// Copyright(c) 2011-2012, Andre Caron (andre.l.caron@gmail.com)
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

/*!
 * @file HttpBasicAuthorizer.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief High-level API for FastCGI application server implementation.
 */

#include "Authorizer.hpp"

#include <b64.hpp>

namespace fcgi {

    /*!
     * @brief Username & password authorizer for HTTP Basic authorization.
     */
    class HttpBasicAuthorizer :
        public Authorizer
    {
        /* contract. */
    protected:
        /*!
         * @brief Specify realm stream transmitted to the browser.
         * @return The realm string.
         *
         * The realm string is presented to the user to inform them of why they
         * need to enter credentials.  The realm string consists of a
         * description of the "area" protected by these credentials.
         */
        virtual const std::string& realm () const = 0;

        /*!
         * @brief Query the password database.
         * @param username Username.
         * @param password Password.
         * @return @c true if the credentials are valid, else @c false.
         */
        virtual bool authorized
            (const std::string& username, const std::string& password) = 0;

        /* overrides. */
    protected:
        virtual void handle_authorization (Request& request)
        {
            // Fetch HTTP Basic authorization token.  Expect this to be empty
            // quite frequently since we need to return a HTTP 401 status
            // before the User Agent even prompts the user for authentication.
            const Headers& headers = request.head();
            const std::string authorization =
                headers.get("HTTP_AUTHORIZATION");
            if (authorization.empty()) {
                errors("No 'Authorization' header.");
                errors();
                output(
                    "Status:401 Authorization required.\r\n"
                    "WWW-Authenticate: Basic realm=\"" + realm() + "\"\r\n"
                    "Content-Type:text/html\r\n"
                    "\r\n"
                    "<p>Enter your credentials for authorization purposes.</p>"
                );
                output();
                end_request();
                return;
            }

            // Fetch credentials, deny all but "basic" authentication.
            std::string credentials;
            { std::istringstream stream(authorization);
                std::string scheme;
                if (!(stream >> scheme) || (scheme != "Basic"))
                {
                    errors(
                        "Authorization scheme '"+scheme+"' not supported."
                    );
                    errors();
                    output(
                        "Status:401 Unsupported authorization method.\r\n"
                        "Content-Type:text/html\r\n"
                        "\r\n"
                        "<h1>Not authorized to access this resource.</h1>"
                    );
                    output();
                    end_request();
                    return;
                }
                if (!(stream >> std::ws) || !std::getline(stream,credentials))
                {
                    errors("Could not read credentials.");
                    errors();
                    output(
                        "Status:401 Invalid credentials.\r\n"
                        "Content-Type:text/html\r\n"
                        "\r\n"
                        "<h1>Not authorized to access this resource.</h1>"
                    );
                    output();
                    end_request();
                    return;
                }
                credentials = b64::decode(credentials);
            }

            // Extract credentials.
            std::string username;
            std::string password;
            std::istringstream stream(credentials);
            if (!(stream >> std::ws) || !std::getline(stream,username,':'))
            {
                errors("Could not extract username.");
                errors();
                output(
                    "Status:401 Invalid credentials.\r\n"
                    "Content-Type:text/html\r\n"
                    "\r\n"
                    "<h1>Not authorized to access this resource.</h1>"
                );
                output();
                end_request();
                return;
            }
            if (!(stream >> std::ws) || !std::getline(stream,password))
            {
                errors("Could not extract password.");
                errors();
                output(
                    "Status:401 Invalid credentials.\r\n"
                    "Content-Type:text/html\r\n"
                    "\r\n"
                    "<h1>Not authorized to access this resource.</h1>"
                );
                output();
                end_request();
                return;
            }

            // Validate credentials.
            if (authorized(username,password))
            {
                errors();
                output(
                    "Status:200 OK.\r\n"
                    "\r\n"
                );
                output();
                end_request();
                return;
            }
            else {
                errors("Invalid credentials:\n");
                errors("  username='"+username+"',\n");
                errors("  password='"+password+"'.\n");
                errors();
                output(
                    "Status:401 Invalid credentials.\r\n"
                    "Content-Type:text/html\r\n"
                    "\r\n"
                    "<h1>Not authorized to access this resource.</h1>"
                );
                output();
                end_request();
                return;
            }
        }
    };

}

#endif /* _fcgi_HttpBasicAuthorizer_hpp__ */
