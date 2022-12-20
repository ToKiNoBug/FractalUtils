project(FractalUtils_fractal_png LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

find_package(PNG)

if(NOT ${PNG_FOUND})
    message(WARNING "libpng not found, fractal_png will not be built.")

    return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/core_utils)

add_library(fractal_png STATIC
    fractal_png.h
    fractal_png.cpp)

target_link_libraries(fractal_png PUBLIC PNG::PNG)

install(TARGETS fractal_png LIBRARY)
install(FILES fractal_png.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)

add_executable(test_fractal_png test_fractal_png.cpp)

target_link_libraries(test_fractal_png PRIVATE fractal_core_utils fractal_png PNG::PNG)