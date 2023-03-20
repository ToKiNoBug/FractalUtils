project(FractalUtils_png_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

# list(APPEND CMAKE_PREFIX_PATH "/home/jeremiah/gcc/gcc12-native")
find_package(PNG)

if((NOT DEFINED PNG_FOUND) OR(NOT ${PNG_FOUND}))
    message(WARNING "libpng not found, png_utils will not be built.")
    return()
endif()

add_library(png_utils STATIC
    png_utils.h
    fractal_png.cpp)
target_link_libraries(png_utils PUBLIC PNG::PNG core_utils)
add_library(fractal_utils::png_utils ALIAS png_utils)

target_compile_features(png_utils PUBLIC cxx_std_17)

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
    INCLUDES DESTINATION include/fractal_utils
)

# install header file
install(FILES png_utils.h DESTINATION include/fractal_utils)

# generate and install export file
install(EXPORT fractal_utils_png_utils-targets
    FILE fractal_utilspng_utilsTargets.cmake
    NAMESPACE fractal_utils::
    DESTINATION lib/cmake/fractal_utils
)

if(NOT ${FractalUtils_build_examples})
    return()
endif()

# example executable
add_executable(test_fractal_png test_fractal_png.cpp)
target_link_libraries(test_fractal_png PRIVATE core_utils png_utils)