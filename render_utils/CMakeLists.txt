project(FractalUtils_render_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

add_library(render_utils STATIC
    render_utils.h render_utils.cpp)

target_link_libraries(render_utils PUBLIC core_utils)
add_library(fractal_utils::render_utils ALIAS render_utils)

target_compile_features(render_utils PUBLIC cxx_std_17)

target_include_directories(render_utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INTERFACE
    $<INSTALL_INTERFACE:include>)

if(NOT ${FractalUtils_no_install})
    install(TARGETS render_utils
        EXPORT fractal_utils_render_utils-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include/fractal_utils)

    # install header file
    install(FILES render_utils.h DESTINATION include/fractal_utils)

    # generate and install export file
    install(EXPORT fractal_utils_render_utils-targets
        FILE fractal_utilsrender_utilsTargets.cmake
        NAMESPACE fractal_utils::
        DESTINATION lib/cmake/fractal_utils
    )
endif()

if(NOT ${FractalUtils_build_examples})
    return()
endif()