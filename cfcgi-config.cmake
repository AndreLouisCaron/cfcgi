# Library configuration file used by dependent projects
# via find_package() built-in directive in "config" mode.

if(NOT DEFINED cfcgi_FOUND)

  # Locate library headers.
  find_path(cfcgi_include_dir
    NAMES fcgi.hpp
    PATHS ${cfcgi_DIR}/code
  )

  # Export library targets.
  set(${PROJECT_NAME}_libraries
    cfcgi
    CACHE INTERNAL "cfcgi library" FORCE
  )

  # Usual "required" et. al. directive logic.
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    cfcgi DEFAULT_MSG
    cfcgi_include_dir
    ${PROJECT_NAME}_libraries
  )

  # Register library targets when found as part of a dependent project.
  # Since this project uses a find_package() directive to include this
  # file, don't recurse back into the CMakeLists file.
  if(NOT ${PROJECT_NAME} STREQUAL cfcgi)
    add_subdirectory(
      ${cfcgi_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/cfcgi
    )
  endif()
endif()
