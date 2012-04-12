/*!
 * @file demo/win/sample-http-basic-authorizer.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief HTTP basic authorization sample.
 */

#include "prespawn-server.hpp"

#include <string>

namespace {

    /*!
     * @brief Sample password database implementation.
     */
    class Application :
        public fcgi::HttpBasicAuthorizer
    {
        /* meta data. */
    public:
        static int concurrent_requests () {
            return (1);
        }

        /* overrides. */
    protected:
        virtual const std::string& realm () const {
            static const std::string realm("Secure Area"); return (realm);
        }

        bool authorized
            (const std::string& username, const std::string& password)
        {
            return ((username == "aladin")     &&
                    (password == "open-sesame"));
        }
    };

}

#include "prespawn-server.cpp"
