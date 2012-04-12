/*!
 * @file demo/win/prespawn-server.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Sample FastCGI server for Windows using the pre-spawn server model.
 *
 * @note This file is designed to be included by a FastCGI application source
 *  file and cannot be compiled by itself.  See the documentation in
 *  @c prespawn-server.hpp for details.
 */

#include "prespawn-server.hpp"

#include <w32.net.hpp>
#include <w32.hpp>

#include <w32/app/prefork-server.hpp>

namespace {

    /*!
     * @brief Implementation of I/O facilities for the application.
     *
     * An instance of this class is created for each incoming request.  Each
     * worker process may own up to @c concurrent_requests() instances of this
     * class at any given time.
     */
    class Session :
        public ::Application
    {
        /* data. */
    private:
        w32::net::tcp::Stream myStream;

        /* construction. */
    public:
        /*!
         * @brief Create a session.
         * @param stream TCP stream socket.
         */
        Session (w32::net::tcp::Stream stream)
            : myStream(stream)
        {}

        /* methods. */
    public:
        void feed (const char * data, size_t size)
        {
            // parse received record(s).
            afeed(data, size);
        }

        /* overrides. */
    protected:
        virtual void asend (const char * data, size_t size)
        {
            // push received data.
            size_t sent = 0;
            size_t pass = 0;
            while ((pass=myStream.put(data+sent,size-sent)) > 0) {
                sent += pass;
            }

            // confirm that we were able to send everything.
            if (sent != size) {
                std::cout
                    << "Failed to send entire payload."
                    << std::endl;
            }
        }

        virtual void query
            (const std::string& name, const std::string& data)
        {
            // ...
            if (concurrent_requests() > 0)
            {
            }
        }
    };

    /*!
     * @brief Slave process context.
     */
    class Server
    {
        /* construction. */
    public:
        Server (int argc, wchar_t ** argv)
        {
        }

        /* operators. */
    public:
        /*!
         * @brief Process an incoming connection.
         * @param stream TCP stream socket.
         * @return @c false for early slave process termination, else @c true.
         */
        bool operator() (w32::net::tcp::Stream stream)
        {
            Session session(stream);

            // feed all incoming data to the parser.
            char data[16*1024];
            for (w32::dword size; (size=stream.get(data,sizeof(data))) > 0;) {
                session.feed(data, size);
            }

            // allow accepting infinite connections.
            return (true);
        }
    };

}

#include <w32/app/prefork-server.cpp>

