project(FractalUtils_core_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_library(core_utils STATIC
    fractal_binfile.h
    fractal_binfile.cpp

    fractal_map.h
    fractal_map.cpp

    fractal_colors.h
    colors.cpp
    color_sources.h
    color_sources.cpp)

# add include directories
set(core_install_headers
    core_utils.h
    fractal_binfile.h
    fractal_colors.h
    fractal_map.h
)

add_library(fractal_utils::core_utils ALIAS core_utils)

# add include directories
target_include_directories(core_utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INTERFACE
    $<INSTALL_INTERFACE:include>
)

# install the target and create export-set
install(TARGETS core_utils
    EXPORT fractal_utils_core-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# install header file
install(FILES ${core_install_headers} DESTINATION include/fractal_utils)

# generate and install export file
install(EXPORT fractal_utils_core-targets
    FILE fractal_utilscore_utilsTargets.cmake
    NAMESPACE fractal_utils::
    DESTINATION lib/cmake/fractal_utils
)

if(NOT ${FractalUtils_build_examples})
    return()
endif()

add_executable(test_binfile test_binfile.cpp)
target_link_libraries(test_binfile PRIVATE core_utils)

add_executable(test_colors test_colors.cpp)
target_link_libraries(test_colors PRIVATE core_utils)