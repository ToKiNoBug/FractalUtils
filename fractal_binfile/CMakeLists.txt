project(FractalUtils_fractal_map LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(fractal_binfile STATIC
    fractal_binfile.h
    fractal_binfile.cpp)