/*!
 * @file demo/nix/prefork-server.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Sample FastCGI server for *NIX using the pre-fork server model.
 */

#include "prefork-server.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <set>

namespace fcgi { namespace nix {

    /*!
     * @brief Implementation of I/O facilities for the application.
     *
     * An instance of this class is created for each incoming request.  Each
     * worker process may own up to @c concurrent_requests instances of this
     * class at any given time.
     */
    class Session :
        public ::Application
    {
        /* data. */
    private:
        int myStream;

        /* construction. */
    public:
        /*!
         * @brief Create a session.
         * @param stream TCP stream socket file descriptor.
         */
        Session (int stream)
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
            ssize_t sent = 0;
            ssize_t pass = 0;
            while ((pass=::send(myStream,data+sent,size-sent,0)) > 0) {
                sent += pass;
            }

            // confirm that we were able to send everything.
            if (sent != size)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to echo entire packet."
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

    class Server; int run (Server& server);

    /*!
     * @brief Master process context.
     */
    class Server
    {
      /* data. */
    private:
      int myListener;
      std::set< ::pid_t > myWorkers;

      /* construction. */
    public:
        Server (int argc, char ** argv)
          : myListener(-1)
        {
            // IPv4 end point to listen on.
            ::sockaddr_in endpoint;
            endpoint.sin_family = AF_INET;
            endpoint.sin_addr.s_addr = INADDR_ANY;
            endpoint.sin_port = htons(9000);

            // Create a listening socket.
            myListener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (myListener == -1)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to create socket: '" << ::strerror(errno) << "'."
                    << std::endl;
                throw (std::exception());
            }

            // Configure the listening socket.
            { const int status = ::bind(myListener,
                                        (::sockaddr*)&endpoint,
                                        sizeof(endpoint));
                if (status == -1)
                {
                    std::cout
                        << "[" << ::getpid() << "] "
                        << "Failed to bind socket: '"
                        << ::strerror(errno) << "'."
                        << std::endl;
                    ::close(myListener);
                    throw (std::exception());
                }
            }

            // Start listening for connections.
            { const int status = ::listen(myListener, SOMAXCONN);
                if (status == -1)
                {
                    std::cout
                        << "[" << ::getpid() << "] "
                        << "Failed to listen: '" << ::strerror(errno) << "'."
                        << std::endl;
                    ::close(myListener);
                    throw (std::exception());
                }
            }
        }

        ~Server ()
        {
            ::close(myListener);
        }

      /* methods. */
    public:
        /*!
         * @brief Obtain the maximum number of requests handled by each worker.
         * @return The maximum number of requests for each worker.
         *
         * This represents the maximum number of total connections, regardless
         * of FastCGI connection multiplexing.  After handling @c worker_quota()
         * connections, the worker process will self-terminate and a new worker
         * process will be spawned to replace it.
         */
        int worker_quota () const {
            return (1);
        }

        /*!
         * @brief Obtain the listener socket.
         * @return The listener socket file descriptor.
         *
         * This listener socket is shared between all worker processes.  It is
         * inherited from the master process when spawning the worker.
         */
        int listener () const {
            return (myListener);
        }

        /*!
         * @brief Block until the gateway attemps to connect.
         * @return The TCP stream socket file descriptor for the new connection.
         */
        int accept_connection ()
        {
            ::sockaddr_in endpoint;
            ::socklen_t size = sizeof(endpoint);
            return (::accept(myListener, (::sockaddr*)&endpoint, &size));
        }

        /*!
         * @brief Obtain the maximum number of simultaneous worker processes.
         * @return The upper bound on the number of worker processes.
         *
         * This value equates to the number of simultaneous open connections
         * because each worker process processes exactly one incoming connection
         * at a time.
         *
         * If FastCGI connection multiplexing is disabled, this also represents
         * the maximum number of simultaneous open requests being processed at
         * any given time.  If connection multiplexing is enabled, multiply this
         * value by the upper bound on the number of simultaneous requests per
         * connection to obtain the total number of simultaneous requests.
         */
        std::size_t workers () const
        {
            return (1);
        }

        /*!
         * @brief Spawn worker processes until there are @c workers().
         *
         * This method should be called as soon as the server is ready to
         * accept incoming connections.
         */
        void spawn_workers ()
        {
            while (myWorkers.size() < workers()) {
                spawn_worker();
            }
        }

        /*!
         * @brief Monitor worker status changes, re-spawn if necessary.
         *
         * This method replaces all worker processes that have exited, then
         * returns control to the caller.  You should call this method in a
         * loop in the master process and stop calling it once it has been
         * determined that the server should stop accepting incoming
         * connections.
         */
        void monitor_workers ()
        {
            int status = 0;
            ::pid_t worker = ::waitpid(-1, &status, WNOHANG);
            while (worker != 0)
            {
                if (worker < 0)
                {
                    std::cout
                        << "[" << ::getpid() << "] "
                        << "Failed to wait: '" << ::strerror(errno) << "'."
                        << std::endl;
                    throw (std::exception());
                }

                if (WIFEXITED(status))
                {
                    const int exit_status = WEXITSTATUS(status);
                    if (exit_status != 0)
                    {
                        std::cout
                            << "[" << ::getpid() << "] "
                            << "Worker failure (" << exit_status << ")."
                            << std::endl;
                    }
                    myWorkers.erase(worker);

                    // Replace the worker.
                    spawn_worker();
                }

                // Check for another status change.
                worker = ::waitpid(-1, &status, WNOHANG);
            }
        }

        /*!
         * @brief Wait for all worker processes to complete, don't respawn.
         *
         * Call this method after it has been determined that no more incoming
         * connections should be accepted.
         *
         * @todo Signal workers to let them know they should stop accepting
         *  incoming connections and return early.
         * @todo Implement some sort of timeout, after which to kill any
         *  leftover workers.  This would enable an upper bound on shutdown.
         */
        void wait_for_workers ()
        {
            while (!myWorkers.empty())
            {
                int status = 0;
                const ::pid_t worker = ::wait(&status);
                if (worker < 0)
                {
                    std::cout
                        << "[" << ::getpid() << "] "
                        << "Failed to wait: '" << ::strerror(errno) << "'."
                        << std::endl;
                    throw (std::exception());
                }
                if (WIFEXITED(status))
                {
                    const int status = WEXITSTATUS(status);
                    if (status != 0)
                    {
                        std::cout
                            << "[" << ::getpid() << "] "
                            << "Worker exited abnormally."
                            << std::endl;
                    }
                    myWorkers.erase(worker);
                }
            }
        }

    private:
        void spawn_worker ()
        {
              // Spawn a new slave process.
            const ::pid_t worker = ::fork();
            if (worker < 0)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to spawn: '" << ::strerror(errno) << "'."
                    << std::endl;
                throw (std::exception());
            }

            // Assign new slave process some work.
            if (worker == 0) {
                std::exit(run(*this));
            }

            std::cout
                << "[" << ::getpid() << "] Spawned! (" << worker << ")"
                << std::endl;

            // Keep track of new slave process.
            myWorkers.insert(worker);
        }
    };

    /*!
     * @brief Slave process context.
     */
    class Worker
    {
        /* data. */
    private:
        int myQuota;
        int myCount;

        /* construction. */
    public:
        Worker (Server& server)
            : myQuota(server.worker_quota()), myCount(0)
        {}

        /* operators. */
    public:
        bool achieved_quota () const {
            return (myCount >= myQuota);
        }

        bool handle (int stream)
        {
            std::cout
                << "[" << ::getpid() << "] "
                << "Creating a session."
                << std::endl;
            Session session(stream);

            std::cout
                << "[" << ::getpid() << "] "
                << "Reading data!"
                << std::endl;
            char data[4*1024];
            ssize_t size = 0;
            while ((size=::recv(stream,data,sizeof(data),0)) > 0)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Received " << size << " bytes."
                    << std::endl;
                session.feed(data, size);
            }

            // Validate that last read suceeded.
            if (size < 0)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to read: '" << ::strerror(errno) << "'."
                    << std::endl;
            }

            // Update worker quota.
            return (++myCount < myQuota);
        }
    };

    /*!
     * @brief Entry point for worker processes.
     */
    int run (Server& server)
    try
    {
        Worker worker(server);

        // Accept connections until the quota is satisfied
        // or the handler requests an early exit.
        int stream = -1;
        do {
            // Wait for connection from remote peer.
            std::cout
                << "[" << ::getpid() << "] "
                << "Waiting for a connection."
                << std::endl;
            stream = server.accept_connection();
            if (stream == -1)
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to accept: '" << ::strerror(errno) << "'."
                    << std::endl;
                return (EXIT_FAILURE);
            }

            // Handle client connection.
            std::cout
                << "[" << ::getpid() << "] "
                << "Received a connection (socket=" << stream << ")."
                << std::endl;
        }
        while (worker.handle(stream));

        // Finished!
        std::cout
            << "[" << ::getpid() << "] "
            << "Over and out";
        if (!worker.achieved_quota()) {
            std::cout << " (early exit)";
        }
        std::cout << "!" << std::endl;

        ::close(stream);
        return (EXIT_SUCCESS);
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "[" << ::getpid() << "] "
            << "Handler failed (" << error.what() << ")."
            << std::endl;
        return (EXIT_FAILURE);
    }
    catch (...)
    {
        std::cerr
            << "[" << ::getpid() << "] "
            << "Handler crashed!"
            << std::endl;
        return (EXIT_FAILURE);
    }

} }

/*!
 * @brief Entry point for master process.
 */
int main (int argc, char ** argv)
try
{
    // Initialize the server.
    fcgi::nix::Server server(argc, argv);

    // Spawn initial set of workers.
    server.spawn_workers();

    // Re-spawn workers as necessary, check 20 times per second.
    const ::useconds_t microsecond = 1;
    const ::useconds_t millisecond = 1000*microsecond;
    while (true) {
        ::usleep(50*millisecond);
        server.monitor_workers();
    }

    // Wait for all workers to complete (don't leave zombies).
    server.wait_for_workers();
}
catch (const std::exception& error)
{
    std::cerr
        << error.what()
        << std::endl;
    return (EXIT_FAILURE);
}
catch (...)
{
    std::cerr
        << "Uncaught internal error!"
        << std::endl;
    return (EXIT_FAILURE);
}
