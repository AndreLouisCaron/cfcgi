# C interface.
set(headers
  fcgi.h
  ipstream.h
  iwire.h
  owire.h
  types.h
)
set(sources
  ipstream.c
  iwire.c
  owire.c
)
add_library(fcgi
  STATIC ${sources} ${headers}
)

# C++ interface.
set(headers
  Application.hpp
  Authorizer.hpp
  fcgi.hpp
  Gateway.hpp
  Headers.hpp
  HttpBasicAuthorizer.hpp
  ostream.hpp
  Request.hpp
  Response.hpp
  Role.hpp
)
set(sources
  Application.cpp
  Gateway.cpp
  Headers.cpp
)
add_library(fcgixx
  STATIC ${sources} ${headers}
)
add_dependencies(fcgixx fcgi b64 b64xx)
target_link_libraries(fcgixx fcgi b64 b64xx)
