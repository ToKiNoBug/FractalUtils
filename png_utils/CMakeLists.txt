project(FractalUtils_png_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# list(APPEND CMAKE_PREFIX_PATH "/home/jeremiah/gcc/gcc12-native")
find_package(PNG)

if((NOT DEFINED PNG_FOUND) OR(NOT ${PNG_FOUND}))
    message(WARNING "libpng not found, png_utils will not be built.")
    return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/core_utils)

add_library(png_utils STATIC
    png_utils.h
    fractal_png.cpp)

target_link_libraries(png_utils PRIVATE PNG::PNG)

add_executable(test_fractal_png test_fractal_png.cpp)

target_link_libraries(test_fractal_png PRIVATE core_utils png_utils PNG::PNG)

add_library(fractal_utils::png_utils ALIAS png_utils)

# add include directories
target_include_directories(png_utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INTERFACE
    $<INSTALL_INTERFACE:include>
)

# install the target and create export-set
install(TARGETS png_utils
    EXPORT fractal_utils_png_utils-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# install header file
install(FILES png_utils.h DESTINATION include)

# generate and install export file
install(EXPORT fractal_utils_png_utils-targets
    FILE fractal_utilspng_utilsTargets.cmake
    NAMESPACE fractal_utils::
    DESTINATION lib/cmake/fractal_utils
)