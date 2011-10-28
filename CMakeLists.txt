#
# FastCGI wire protocol implementation in C.
#

cmake_minimum_required(VERSION 2.6)

project(cfcgi)

  # base paths to source files.
set(fcgi_path ${CMAKE_CURRENT_SOURCE_DIR}/code)
set(demo_path ${CMAKE_CURRENT_SOURCE_DIR}/demo)

  # core library and c++ wrappers.
file(GLOB fcgi_headers ${fcgi_path}/*.h ${fcgi_path}/*.hpp)
file(GLOB fcgi_sources ${fcgi_path}/*.c ${fcgi_path}/*.cpp)
add_library(fcgi
  STATIC ${fcgi_sources} ${fcgi_headers})

  # demo program.
include_directories(${fcgi_path})
set(demo_sources
  ${demo_path}/demo.cpp)
add_executable(demo ${demo_sources})
target_link_libraries(demo fcgi)

  # make sure fcgi library is built first.
add_dependencies(demo fcgi)
