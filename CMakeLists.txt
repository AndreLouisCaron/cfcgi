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

# set global compiler options.
set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/site
)
find_package(MSVCExtras)
msvc_configure_runtime()
msvc_enable_se_handling()

# build dependencies.
set(cb64_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/cb64
)
find_package(cb64)

if(MSVC)
  # Ignore warnings about using standard functions.
  add_definitions(
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
  )
  # Target a specific Windows version (Vista and above).
  add_definitions(
    -D_WIN32_WINNT=0x601
  )
endif()

# resolve library headers.
include_directories(
  ${cb64_include_dirs}
)

# build main library.
add_subdirectory(code)

# exclude all but the core library when building as a dependency.
if(${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})

  # resolve our own library headers.
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/code
  )

  # extra dependencies for Windows-specific demo projects.
  if(WIN32)
    set(w32_DIR
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/w32
    )
    set(w32_libraries
      w32
      w32.dbg
      w32.io
      w32.ipc
      w32.mt
      w32.net
      w32.net.tcp
      w32.net.ipv4
    )
    find_package(w32
      COMPONENTS ${w32_libraries}
    )
  endif()

  # build demo projects.
  add_subdirectory(demo)

  # add 'help' target for API documentation.
  find_package(doxygen)
  find_package(DoxygenExtras)
  configure_file(
    ${CMAKE_SOURCE_DIR}/help/Doxyfile
    ${CMAKE_BINARY_DIR}/Doxyfile @ONLY
  )
  add_api_documentation(help)

endif()
