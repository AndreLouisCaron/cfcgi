# FastCGI wire protocol implementation in C.
#
# If building the library as a standalone project, demo p
#
# Extra targets:
#   help: builds API documentation in HTML format using Doxygen.  Add the
#         "-DPROJECT_VERSION=..." flag to the CMake command when publishing
#         the docuentation for a specific version.  This target is only
#         available if building the library as a standalone project.  It
#         is not available if added to a dependent project using the
#         "find_package(cfcgi)" directive.

cmake_minimum_required(VERSION 2.6)

project(cfcgi)

find_package(doxygen)

# Compile API documentation from source code.
function(add_api_documentation target)
  if(DOXYGEN_EXECUTABLE)
    add_custom_target(
      ${target}
      COMMAND
        ${DOXYGEN_EXECUTABLE}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT
        "Compiling documentation."
      VERBATIM
    )
  endif()
endfunction()

# build dependencies.
set(cb64_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/cb64
)
find_package(cb64)

# resolve library headers.
include_directories(${cb64_include_dirs})

# build main library.
add_subdirectory(code)

# exclude all but the core library when building as a dependency.
if(${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})

  # resolve our own library headers.
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/code
  )

  # build demo projects.
  add_subdirectory(demo)

  # add 'help' target for API documentation.
  configure_file(
    ${CMAKE_SOURCE_DIR}/help/Doxyfile
    ${CMAKE_BINARY_DIR}/Doxyfile @ONLY
  )
  add_api_documentation(help)

endif()
