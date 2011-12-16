#
# FastCGI wire protocol implementation in C.
#

cmake_minimum_required(VERSION 2.6)

project(cfcgi)

  # base paths to source files.
set(b64_path ${CMAKE_CURRENT_SOURCE_DIR}/libs/cb64/code)
set(fcgi_path ${CMAKE_CURRENT_SOURCE_DIR}/code)
set(demo_path ${CMAKE_CURRENT_SOURCE_DIR}/demo)

  # core library and c++ wrappers.
file(GLOB fcgi_headers ${fcgi_path}/*.h ${fcgi_path}/*.hpp)
file(GLOB fcgi_sources ${fcgi_path}/*.c ${fcgi_path}/*.cpp)
add_library(fcgi
  STATIC ${fcgi_sources} ${fcgi_headers})

  # build base-64 library.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/cb64)

  # only build demo program(s) if this is a standalone build.
string(COMPARE EQUAL
  ${PROJECT_NAME} ${CMAKE_PROJECT_NAME}
  ${PROJECT_NAME}_is_toplevel)
if(${${PROJECT_NAME}_is_toplevel})

    # simple demonstration.
  include_directories(${b64_path})
  include_directories(${fcgi_path})
  set(demo_sources
    ${demo_path}/demo.cpp)
  add_executable(demo ${demo_sources})
  target_link_libraries(demo fcgi)
  add_dependencies(demo fcgi)

    # pre-fork authorizer application.
  if(UNIX)
    set(sources
      ${demo_path}/nix/prefork-server.cpp)
    add_executable(prefork-authorizer ${sources})
    target_link_libraries(prefork-authorizer fcgi b64)
    add_dependencies(prefork-authorizer fcgi b64)
  endif()

endif()
