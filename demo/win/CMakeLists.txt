include_directories(${w32_include_dir})
set(sources
  sample-http-basic-authorizer.cpp
)
add_executable(prefork-authorizer ${sources})
message(STATUS "Win32 libraries: '${w32_libraries}'.")
target_link_libraries(prefork-authorizer
  fcgi fcgixx b64 b64xx ${w32_libraries}
)
add_dependencies(prefork-authorizer
  fcgi fcgixx b64 b64xx ${w32_libraries}
)
