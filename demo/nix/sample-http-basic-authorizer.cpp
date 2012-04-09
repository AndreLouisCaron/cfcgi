/*!
 * @file demo/nix/sample-http-basic-authorizer.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief HTTP basic authorization sample.
 */

#include "prefork-server.hpp"

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
        bool authorized
            (const std::string& username, const std::string& password)
        {
            return ((username == "aladin")     &&
                    (password == "open-sesame"));
        }
    };

}

#include "prefork-server.cpp"
