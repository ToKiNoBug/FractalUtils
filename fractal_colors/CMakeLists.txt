project(FractalUtils_fractal_colors LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(colors STATIC
    colors.h
    colors.cpp
    color_sources.h
    color_sources.cpp)