project(FractalUtils_fractal_core_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

add_library(fractal_core_utils STATIC
    fractal_binfile.h
    fractal_binfile.cpp

    fractal_map.h
    fractal_map.cpp

    fractal_colors.h
    colors.cpp
    color_sources.h
    color_sources.cpp)

set(core_utils_install_headers
    fractal_binfile.h
    fractal_map.h
    fractal_colors.h
    core_utils.h)

install(TARGETS fractal_core_utils LIBRARY)
install(FILES ${core_utils_install_headers}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/fractal_utils)

add_executable(test_binfile test_binfile.cpp)

target_link_libraries(test_binfile PRIVATE fractal_core_utils)

add_executable(test_colors test_colors.cpp)

target_link_libraries(test_colors PRIVATE fractal_core_utils)