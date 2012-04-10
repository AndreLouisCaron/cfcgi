# Simple demo.
set(sources
  demo.cpp
)
add_executable(demo ${sources})
target_link_libraries(demo fcgi fcgixx)
add_dependencies(demo fcgi fcgixx)

# Platform-specific demos.
if(UNIX)
  add_subdirectory(nix)
endif()
