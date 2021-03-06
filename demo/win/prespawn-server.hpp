#ifndef _demo_win_prespawn_server_hpp__
#define _demo_win_prespawn_server_hpp__

/*!
 * @file demo/win/prespawn-server.hpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Sample FastCGI server for Windows using the pre-spawn server model.
 */

#include <fcgi.hpp>

namespace {

    /*!
     * @brief FastCGI application handler.
     *
     * This class must be implemented in order to use the built-in pre-fork
     * server model.  For example, the skeleton for a FastCGI responder looks
     * like:
     *
     * @code
     *  #include "prefork-server.hpp"
     *  namespace {
     *    class Application :
     *        public fcgi::Responder
     *    {
     *      // ...
     *    };
     *  }
     *  #include "prefork-server.cpp"
     * @endcode
     *
     * This class must inherit from @c fcgi::Authorizer, @c fcgi::Responder
     * or @c fcgi::Filter.  It must implement any abstract methods and respect
     * the contract of the selected base class.
     */
    class Application;

}

#endif /* _demo_win_prespawn_server_hpp__ */
