set(sources
  sample-http-basic-authorizer.cpp
)
add_executable(prefork-authorizer ${sources})
target_link_libraries(prefork-authorizer fcgi fcgixx b64)
add_dependencies(prefork-authorizer fcgi fcgixx b64)
