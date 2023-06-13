project(FractalUtils_video_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

find_package(fmt REQUIRED)
find_package(OpenMP REQUIRED)

add_library(video_utils STATIC
        video_utils.h
        video_utils.cpp)
target_compile_features(video_utils PUBLIC cxx_std_20)
target_link_libraries(video_utils PUBLIC
        core_utils
        fmt::fmt
        OpenMP::OpenMP_CXX)
target_include_directories(video_utils
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>)

set(video_utils_install_headers
        video_utils.h)

add_library(fractal_utils::video_utils ALIAS video_utils)


if (NOT ${FractalUtils_no_install})
    # install the target and create export-set
    install(TARGETS video_utils
            EXPORT fractal_utils_video_utils-targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include/fractal_utils
            )

    # install header file
    install(FILES ${video_utils_install_headers} DESTINATION include/fractal_utils)

    # generate and install export file
    install(EXPORT fractal_utils_video_utils-targets
            FILE fractal_utilsvideo_utilsTargets.cmake
            NAMESPACE fractal_utils::
            DESTINATION lib/cmake/fractal_utils
            )
endif ()

if (NOT ${FractalUtils_build_examples})
    return()
endif ()