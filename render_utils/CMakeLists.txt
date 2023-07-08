project(FractalUtils_render_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(render_utils_install_headers
        render_utils.h
        color_cvt.hpp
        color_cvt_cuda.hpp)

add_library(render_utils STATIC
        ${render_utils_install_headers}
        render_utils.cpp)

target_link_libraries(render_utils PUBLIC core_utils)
add_library(fractal_utils::render_utils ALIAS render_utils)

target_compile_features(render_utils PUBLIC cxx_std_17)

target_include_directories(render_utils
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>)

#message(STATUS "Searching for nvcc compiler...")
#execute_process(COMMAND nvcc --version
#        ERROR_VARIABLE fu_nvcc_error)
#
#
#if(NOT fu_nvcc_error)
#    set(FractalUtils_cuda_support ON)
#    enable_language(CUDA)
#else()
#    message(STATUS "nvcc not found, cuda extension is disabled.")
#    set(FractalUtils_cuda_support OFF)
#endif ()


if (NOT ${FractalUtils_no_install})
    install(TARGETS render_utils
            EXPORT fractal_utils_render_utils-targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include/fractal_utils)

    # install header file
    install(FILES ${render_utils_install_headers}
            DESTINATION include/fractal_utils)

    # generate and install export file
    install(EXPORT fractal_utils_render_utils-targets
            FILE fractal_utilsrender_utilsTargets.cmake
            NAMESPACE fractal_utils::
            DESTINATION lib/cmake/fractal_utils
            )
endif ()

if (NOT ${FractalUtils_build_examples})
    return()
endif ()

add_executable(test_color_cvt test_color_cvt.cpp)
target_link_libraries(test_color_cvt PRIVATE render_utils)

#if(${FractalUtils_cuda_support})
#    #add_executable(test_render_utils_cuda_extension test_render_utils_cuda_extension.cu)
#    #target_link_libraries(test_render_utils_cuda_extension PRIVATE render_utils)
#endif()