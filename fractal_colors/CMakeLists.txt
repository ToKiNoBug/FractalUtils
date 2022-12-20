project(FractalUtils_fractal_colors LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(fractal_colormap STATIC
    colors.h
    colors.cpp
    color_sources.h
    color_sources.cpp)

install(TARGETS fractal_colormap LIBRARY)
install(FILES colors.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)

add_executable(test_colors test_colors.cpp)

target_link_libraries(test_colors PRIVATE fractal_colormap)