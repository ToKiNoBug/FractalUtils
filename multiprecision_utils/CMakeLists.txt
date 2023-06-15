project(FractalUtils_multiprecision_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)


find_package(Boost COMPONENTS multiprecision)

set(FractalUtils_mp_boost_mp_target_name)
unset(FractalUtils_mp_boost_include_dir)
if(NOT ${BOOST_FOUND})
    message(STATUS "Failed to find Boost::multiprecision by find_package, searching for its headers")
    find_file(boost_multiprecision_header_file cpp_bin_float.hpp
            PATH_SUFFIXES include/boost/multiprecision)
    if(boost_multiprecision_header_file)
        message(STATUS "Found include/boost/multiprecision/cpp_bin_float.hpp at ${boost_multiprecision_header_file}")
        cmake_path(GET boost_multiprecision_header_file PARENT_PATH temp)
        cmake_path(GET temp PARENT_PATH temp)
        cmake_path(GET temp PARENT_PATH temp)
        set(FractalUtils_mp_boost_include_dir ${temp})

        message(STATUS "FractalUtils_mp_boost_include_dir = ${FractalUtils_mp_boost_include_dir}")
        #add_library(FractalUtils_interface_target_for_boost_multiprecision INTERFACE)
        #target_include_directories(FractalUtils_interface_target_for_boost_multiprecision INTERFACE ${temp})
        #set(FractalUtils_mp_boost_mp_target_name FractalUtils_interface_target_for_boost_multiprecision)
        set(BOOST_FOUND TRUE)
    endif()

else()
    set(FractalUtils_mp_boost_mp_target_name "Boost::multiprecision")
endif()

if(NOT ${BOOST_FOUND})
    message(WARNING "Boost::multiprecision is not found, so multiprecision_utils will be disabled.")
    return()
endif()


# add include directories
set(multiprecision_install_headers
        multiprecision_utils.h
        mp_ints.hpp
        mp_floats.hpp
        encode.hpp
        decode.hpp
        )

add_library(multiprecision_utils INTERFACE
        ${multiprecision_install_headers})


target_link_libraries(multiprecision_utils INTERFACE
        ${FractalUtils_mp_boost_mp_target_name}
        )


add_library(fractal_utils::multiprecision_utils ALIAS multiprecision_utils)

target_compile_features(multiprecision_utils INTERFACE cxx_std_20)

# add include directories
target_include_directories(multiprecision_utils
        #PUBLIC
        #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>
        ${FractalUtils_mp_boost_include_dir}
        )

if(NOT ${FractalUtils_no_install})
    # install the target and create export-set
    install(TARGETS multiprecision_utils
            EXPORT fractal_utils_multiprecision-targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
            )

    # install header file
    install(FILES ${multiprecision_install_headers} DESTINATION include/fractal_utils)

    # generate and install export file
    install(EXPORT fractal_utils_multiprecision-targets
            FILE fractal_utilsmultiprecision_utilsTargets.cmake
            NAMESPACE fractal_utils::
            DESTINATION lib/cmake/fractal_utils
            )
endif()

if(NOT ${FractalUtils_build_examples})
    return()
endif()

add_executable(test_mpfloat_cvt test_mpfloat_cvt.cpp)
target_link_libraries(test_mpfloat_cvt PRIVATE multiprecision_utils)