project(FractalUtils_fractal_map LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(fractal_map STATIC
    fractal_map.h
    fractal_map.cpp)

install(TARGETS fractal_map LIBRARY)
install(FILES fractal_map.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)