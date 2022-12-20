project(FractalUtils_fractal_map LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(fractal_binfile STATIC
    fractal_binfile.h
    fractal_binfile.cpp)

add_executable(test_binfile test_binfile.cpp)

target_link_libraries(test_binfile PRIVATE fractal_binfile)

install(TARGETS fractal_binfile LIBRARY)
install(FILES fractal_binfile.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)