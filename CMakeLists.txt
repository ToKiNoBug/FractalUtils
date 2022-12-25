project(FractalUtils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
endif()

include(add_compiler_path_to_prefix.cmake)

add_subdirectory(core_utils)
add_subdirectory(png_utils)

# include CMakePackageConfigHelpers macro
include(CMakePackageConfigHelpers)

# set version
set(version 1.0.7)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config-version.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config.cmake"
    INSTALL_DESTINATION lib/cmake/fractal_utils
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/fractal_utils-config-version.cmake"
    DESTINATION lib/cmake/fractal_utils
)