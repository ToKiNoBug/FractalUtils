project(FractalUtils_multiprecision_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)


find_package(Boost COMPONENTS multiprecision)

set(FractalUtils_mp_boost_mp_target_name)
unset(FractalUtils_mp_boost_include_dir)
if (NOT ${BOOST_FOUND})
    message(STATUS "Failed to find Boost::multiprecision by find_package, searching for its headers")
    find_file(boost_multiprecision_header_file "cpp_bin_float.hpp"
            PATH_SUFFIXES include/boost/multiprecision)
    if (boost_multiprecision_header_file)
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
    endif ()

else ()
    set(FractalUtils_mp_boost_mp_target_name "Boost::multiprecision")
endif ()

if (NOT ${BOOST_FOUND})
    message(WARNING "Boost::multiprecision is not found, so multiprecision_utils will be disabled.")
    return()
endif ()


unset(FractalUtils_optional_sources)
set(FractalUtils_multiprecision_gmp_support false)
set(FractalUtils_multiprecision_mpfr_support false)

find_package(GMP QUIET)
if (GMP_FOUND OR LINUX)
    find_file(gmp_header_file "gmp.h")
    if (gmp_header_file)
        cmake_path(GET gmp_header_file PARENT_PATH gmp_include_dir)
        list(APPEND FractalUtils_optional_sources
                gmp_support.h
                gmp_support.cpp)
        set(FractalUtils_multiprecision_gmp_support true)
    else ()
        unset(gmp_include_dir)
        message(WARNING "gmp should exist on this platform, but gmp.h is not found. gmp support will be disabled.")
    endif ()
endif ()

find_package(MPFR QUIET)
if (MPFR_FOUND OR LINUX)
    find_file(mpfr_header_file "mpfr.h")
    if (mpfr_header_file)
        cmake_path(GET mpfr_header_file PARENT_PATH mpfr_include_dir)
        list(APPEND FractalUtils_optional_sources
                mpfr_support.h
                mpfr_support.cpp)
        set(FractalUtils_multiprecision_mpfr_support true)
    else ()
        unset(mpfr_include_dir)
        set(FractalUtils_multiprecision_mpfr_support false)
        message(WARNING "mpfr should exist on this platform, but mpfr.h is not found. mpfr support will be disabled.")
    endif ()
endif ()

find_package(MPC QUIET)
if (MPC_FOUND OR LINUX)
    find_file(mpc_header_file "mpc.h")
    if (mpc_header_file)
        cmake_path(GET mpc_header_file PARENT_PATH mpc_include_dir)

        set(FractalUtils_multiprecision_mpc_support true)
    else ()
        unset(mpc_include_dir)
        set(FractalUtils_multiprecision_mpc_support false)
        message(WARNING "mpc should exist on this platform, but mpc.h is not found. mpc support will be disabled.")
    endif ()

endif ()


# add include directories
set(multiprecision_install_headers
        multiprecision_utils.h
        mp_ints.hpp
        mp_floats.hpp
        encode.hpp
        decode.hpp
        mp_complex.hpp

        gmp_support.h
        mpfr_support.h
        mpc_support.h)

message(STATUS "FractalUtils_optional_sources = ${FractalUtils_optional_sources}")

add_library(multiprecision_utils STATIC
        ${multiprecision_install_headers}
        empty.cpp
        ${FractalUtils_optional_sources}
        )


target_link_libraries(multiprecision_utils PUBLIC
        ${FractalUtils_mp_boost_mp_target_name}
        core_utils
        )


target_compile_features(multiprecision_utils PUBLIC cxx_std_20)

# add include directories
target_include_directories(multiprecision_utils
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${FractalUtils_mp_boost_include_dir}
        ${gmp_include_dir}
        ${mpfr_include_dir}
        INTERFACE
        $<INSTALL_INTERFACE:include>
        )

if (${FractalUtils_multiprecision_gmp_support})
    target_compile_definitions(multiprecision_utils PUBLIC
            FRACTALUTILS_MULTIPRECISIONUTILS_GMP_SUPPORT=1)
endif ()
if (${FractalUtils_multiprecision_mpfr_support})
    target_compile_definitions(multiprecision_utils PUBLIC
            FRACTALUTILS_MULTIPRECISIONUTILS_MPFR_SUPPORT=1)
endif ()
if (${FractalUtils_multiprecision_mpc_support})
    target_compile_definitions(multiprecision_utils PUBLIC
            FRACTALUTILS_MULTIPRECISIONUTILS_MPC_SUPPORT=1)
endif ()


add_library(fractal_utils::multiprecision_utils ALIAS multiprecision_utils)


if (NOT ${FractalUtils_no_install})
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
endif ()

if (${FractalUtils_build_examples})
    add_executable(test_boost_float test_boost_float.cpp)
    target_link_libraries(test_boost_float PRIVATE multiprecision_utils)
endif ()


if ((${LINUX}) OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))

    if (${FractalUtils_build_examples})
        add_executable(test_quadmath test_quadmath.cpp)
        target_link_libraries(test_quadmath PRIVATE
                multiprecision_utils
                core_utils)
    endif ()
endif ()


if (${FractalUtils_multiprecision_gmp_support})
    target_link_libraries(multiprecision_utils PUBLIC gmp)


    if (${FractalUtils_build_examples})
        add_executable(test_gmp_float test_gmp_float.cpp)
        target_link_libraries(test_gmp_float PRIVATE
                multiprecision_utils
                core_utils)

        add_executable(test_gmp_complex test_gmp_complex.cpp)
        target_link_libraries(test_gmp_complex PRIVATE
                multiprecision_utils)
    endif ()
endif ()

if (${FractalUtils_multiprecision_mpfr_support})

    target_link_libraries(multiprecision_utils PUBLIC mpfr)

    if (${FractalUtils_build_examples})
        add_executable(test_mpfr_float test_mpfr_float.cpp)
        target_link_libraries(test_mpfr_float PRIVATE
                multiprecision_utils
                core_utils)
        add_executable(test_mpfr_required_prec test_mpfr_required_prec.cpp)
        target_link_libraries(test_mpfr_required_prec PRIVATE
                multiprecision_utils
                core_utils)
    endif ()

endif ()

if (${FractalUtils_multiprecision_mpc_support})

    target_link_libraries(multiprecision_utils PUBLIC mpc)
endif ()
