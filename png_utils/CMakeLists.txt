project(FractalUtils_png_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# list(APPEND CMAKE_PREFIX_PATH "/home/jeremiah/gcc/gcc12-native")
find_package(PNG)

if((NOT DEFINED PNG_FOUND) OR(NOT ${PNG_FOUND}))
    message(WARNING "libpng not found, fractal_png_utils will not be built.")
    return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/core_utils)

add_library(fractal_png_utils STATIC
    png_utils.h
    fractal_png.cpp)

target_link_libraries(fractal_png_utils PRIVATE PNG::PNG)

add_executable(test_fractal_png test_fractal_png.cpp)

target_link_libraries(test_fractal_png PRIVATE fractal_core_utils fractal_png_utils PNG::PNG)

# make package
include(CMakePackageConfigHelpers)

target_include_directories(fractal_png_utils
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS fractal_png_utils
    EXPORT fractal_utils_targets
    LIBRARY ARCHIVE RUNTIME INCLUDES)

install(FILES png_utils.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)
