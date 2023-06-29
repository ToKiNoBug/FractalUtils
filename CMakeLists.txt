
cmake_minimum_required(VERSION 3.15)
project(FractalUtils VERSION 2.3.10 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  set(FractalUtils_is_sub_project OFF)
  set(FractalUtils_is_not_sub_project ON)
else()
  set(FractalUtils_is_sub_project ON)
  set(FractalUtils_is_not_sub_project OFF)
endif()

option(FractalUtils_no_install "Disable installation for this lib." ${FractalUtils_is_sub_project})
option(FractalUtils_no_test "Disable testing for this lib." ${FractalUtils_is_sub_project})
option(FractalUtils_build_examples "Build examples" ${FractalUtils_is_not_sub_project})

if(${FractalUtils_no_install})
  message(STATUS "Installation is disabled for FractalUtils, this is designed to be used as an external project")
endif()

if(NOT ${FractalUtils_no_test})
  enable_testing()
endif()

# set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
endif()

if(${MSVC})
  add_compile_options("/EHsc")
endif()

include(cmake/add_compiler_path_to_prefix.cmake)
include(cmake/configure_3rdparty.cmake)

# message(STATUS "CMAKE_CXX_COMPILER_ID = " ${CMAKE_CXX_COMPILER_ID})
# message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT = "
# ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  OR(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
  message(STATUS "The compiler is MSVC or Clang-cl.")
  add_compile_options(/wd4267 /wd4244 /wd4305)
  message(STATUS "Added msvc compile options to avoid stupid warnings.")
endif()

add_subdirectory(core_utils)
add_subdirectory(render_utils)
add_subdirectory(png_utils)
add_subdirectory(zoom_utils)
add_subdirectory(video_utils)
add_subdirectory(multiprecision_utils)

# include CMakePackageConfigHelpers macro
include(CMakePackageConfigHelpers)

# set version
set(version ${PROJECT_VERSION})

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config-version.cmake"
  VERSION "${version}"
  COMPATIBILITY AnyNewerVersion)

# create config file
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config.cmake"
  INSTALL_DESTINATION lib/cmake/fractal_utils
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

# install config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config-version.cmake"
  DESTINATION lib/cmake/fractal_utils)
