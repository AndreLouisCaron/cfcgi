/*!
 * @file demo/nix/prefork-server.cpp
 * @author Andre Caron (andre.l.caron@gmail.com)
 * @brief Sample FastCGI server for *NIX using the pre-fork server model.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <b64.hpp>
#include <fcgi.hpp>

namespace {

    /*!
     * @brief Implementation of application logic.
     */
    class Authorizer :
        public fcgi::Application
    {
        /* meta data. */
    public:
        static const int concurrent_requests = 1;

        /* contract. */
    protected:
        virtual bool authorized
            ( const std::string& username, const std::string& password ) = 0;

        /* overrides. */
    protected:
        virtual void end_of_head ( fcgi::Request& request )
        {
              // Make sure we're responding to an authorization request.
	    std::cout << "Got a request!" << std::endl;
            if ( request.role() != fcgi::Role::authorizer() )
            {
		std::cout << "Not an authorizer..." << std::endl;
                fcgi::Application::errors(
                    "This is an authorizer, not a responder or filter.");
                fcgi::Application::errors();
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
	    std::cout << "Is an authorizer!" << std::endl;
              // fetch authorization request.
            const fcgi::Headers& headers = request.head();
            const std::string authorization =
                headers.get("HTTP_AUTHORIZATION");
            std::cout
                << "Authorization: '" << authorization << "'."
                << std::endl;
            if ( authorization.empty() ) {
                fcgi::Application::errors("No 'Authorization' header.");
                fcgi::Application::errors();
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
              // Fetch credentials, deny anything not "basic" authentication.
            std::string credentials;
            { std::istringstream stream(authorization);
                std::string scheme;
                if (!(stream >> scheme) || (scheme != "Basic"))
                {
                    fcgi::Application::errors(
                        "Authorization scheme '"+scheme+"' not supported.");
                    fcgi::Application::errors();
                    fcgi::Application::output();
                    fcgi::Application::end_request(1);
                    return;
                }
                if (!(stream >> std::ws) || !std::getline(stream,credentials))
                {
                    fcgi::Application::errors(
                        "Could not read credentials...");
                    fcgi::Application::errors();
                    fcgi::Application::output();
                    fcgi::Application::end_request(1);
                    return;
                }
                std::cout
                    << "Credentials: '" << credentials << "'."
                    << std::endl;
                credentials = b64::decode(credentials);
            }
              // extract credentials.
            std::string username;
            std::string password;
            std::istringstream stream(credentials);
            if ( !(stream >> std::ws) || !std::getline(stream,username,':'))
            {
                fcgi::Application::errors("Could not extract username.");
                fcgi::Application::errors();
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
            if ( !(stream >> std::ws) || !std::getline(stream,password))
            {
                fcgi::Application::errors("Could not extract password.");
                fcgi::Application::errors();
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
            std::cout
                << "Username: '" << username << "'."
                << std::endl;
            std::cout
                << "Password: '" << password << "'."
                << std::endl;
              // validate credentials.
            if ( authorized(username,password) )
            {
                std::cout
                    << "Logged in!"
                    << std::endl;
                fcgi::Application::errors();
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
            else {
                std::cout
                    << "Invalid credentials!"
                    << std::endl;
                fcgi::Application::errors("Invalid credentials:\n");
                fcgi::Application::errors("  username='"+username+"',\n");
                fcgi::Application::errors("  password='"+password+"'.\n");
                fcgi::Application::errors();
                fcgi::Application::output("<h1>Unauthorized.</h1>");
                fcgi::Application::output();
                fcgi::Application::end_request(1);
                return;
            }
        }

        virtual void body ( fcgi::Request& request )
        {
            // received input data.  keep responding.
            // ...
        }

        virtual void end_of_body ( fcgi::Request& request )
        {
            // end of request, send reamaining data now!
            // ...
        }
    };

    class Application :
        public Authorizer
    {
        /* meta data. */
    public:
        static const int concurrent_requests = 1;

        /* overrides. */
    protected:
        bool authorized
            ( const std::string& username, const std::string& password )
        {
            return ((username == "aladin")     &&
                    (password == "open-sesame"));
        }
    };

    /*!
     * @brief Implementation of I/O facilities for the application.
     */
    class Session :
        public Application
    {
        /* data. */
    private:
        int myStream;

        /* construction. */
    public:
        Session ( int stream )
            : myStream(stream)
        {}

        /* methods. */
    public:
        void feed ( const char * data, size_t size )
        {
              // parse received record(s).
            fcgi::Application::afeed(data, size);
        }

        /* overrides. */
    protected:
        virtual void asend ( const char * data, size_t size )
        {
              // push received data.
            ssize_t sent = 0;
            ssize_t pass = 0;
            while ( (pass=::send(myStream,data+sent,size-sent,0)) > 0 ) {
                sent += pass;
            }
              // confirm that we were able to send everything.
            if ( sent != size )
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to echo entire packet."
                    << std::endl;
            }
        }

        virtual void query
            ( const std::string& name, const std::string& data )
        {
            // ...
            if ( Application::concurrent_requests > 0 )
            {
            }
        }
    };

    /*!
     * @brief Process context.
     */
    class Server
    {
        /* data. */
    private:
        int myQuota;
        int myStage;

        /* construction. */
    public:
        Server ( int quota )
            : myQuota(quota), myStage(0)
        {}

        /* operators. */
    public:
        bool operator() ( int stream )
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
            while ( (size=::recv(stream,data,sizeof(data),0)) > 0 )
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Received " << size << " bytes."
                    << std::endl;
                session.feed(data, size);
            }
            if ( size < 0 )
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Failed to read: '" << ::strerror(errno) << "'."
                    << std::endl;
            }
            return (++myStage < myQuota);
        }
    };

    int accept_connection ( int listener )
    {
        ::sockaddr_in endpoint;
        ::socklen_t size = sizeof(endpoint);
        return (::accept(listener, (::sockaddr*)&endpoint, &size));
    }

    /*!
     * @brief Entry point for worker processes.
     */
    int run ( int, char **, int listener )
    try
    {
        Server server(1);
          // Wait for connection from remote peer.
        std::cout
            << "[" << ::getpid() << "] "
            << "Waiting for a connection."
            << std::endl;
        const int stream = ::accept_connection(listener);
        if ( stream == -1 )
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
            << "Received a connection (socket=" << socket << ")."
            << std::endl;
        server(stream);
          // Finished!
        std::cout
            << "[" << ::getpid() << "] "
            << "Over and out!"
            << std::endl;
        ::close(stream);
        ::close(listener);
        return (EXIT_SUCCESS);
    }
    catch ( ... )
    {
        std::cerr
            << "[" << ::getpid() << "] "
            << "Worker crashed."
            << std::endl;
        return (EXIT_FAILURE);
    }

}

/*!
 * @brief Entry point for master process.
 */
int main ( int argc, char ** argv )
{
    const int multiplicity = 1;
      // IPv4 end point to listen on.
    ::sockaddr_in endpoint;
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = INADDR_ANY;
    endpoint.sin_port = ::htons(9000);
      // Create a listening socket.
    const int listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( listener == -1 )
    {
        std::cout
            << "[" << ::getpid() << "] "
            << "Failed to create socket: '" << ::strerror(errno) << "'."
            << std::endl;
        return (EXIT_FAILURE);
    }
    { const int status = ::bind(listener,
                                (::sockaddr*)&endpoint, sizeof(endpoint));
        if ( status == -1 )
        {
            std::cout
                << "[" << ::getpid() << "] "
                << "Failed to bind socket: '" << ::strerror(errno) << "'."
                << std::endl;
            ::close(listener);
            return (EXIT_FAILURE);
        }
    }
    { const int status = ::listen(listener, SOMAXCONN);
        if ( status == -1 )
        {
            std::cout
                << "[" << ::getpid() << "] "
                << "Failed to listen: '" << ::strerror(errno) << "'."
                << std::endl;
            ::close(listener);
            return (EXIT_FAILURE);
        }
    }
      // Spawn workers.
    ::pid_t workers[multiplicity] = { 0 };
    for ( int i = 0; (i < multiplicity); ++i )
    {
          // Spawn one worker.
        workers[i] = ::fork();
        if ( workers[i] < 0 )
        {
            std::cout
                << "[" << ::getpid() << "] "
                << "Failed to spawn: '" << ::strerror(errno) << "'."
                << std::endl;
            ::close(listener);
            return (EXIT_FAILURE);
        }
          // Dispatch actual work to worker process.
        if ( workers[i] == 0 ) {
            return (::run(argc, argv, listener));
        }
          // Keep spawning...
        std::cout
            << "[" << ::getpid() << "] "
            << "Spawned! (" << workers[i] << ")"
            << std::endl;
    }
      // Wait for workers to complete.
    for ( int i = 0; (i < multiplicity); ++i )
    {
        int result = 0;
        const ::pid_t worker = ::wait(&result);
        if ( worker < 0 )
        {
            std::cout
                << "[" << ::getpid() << "] "
                << "Failed to wait: '" << ::strerror(errno) << "'."
                << std::endl;
            ::close(listener);
            return (EXIT_FAILURE);
        }
        if (WIFEXITED(result))
        {
            const int status = WEXITSTATUS(result);
            if ( status != 0 )
            {
                std::cout
                    << "[" << ::getpid() << "] "
                    << "Worker exited abnormally."
                    << std::endl;
            }
        }
    }
      // Finished!
    ::close(listener);
    return (EXIT_SUCCESS);
}
