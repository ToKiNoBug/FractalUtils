project(FractalUtils_zoom_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets)

if(NOT ${QT_FOUND})
  message(WARNING "Qt6 Widgets not found. zoom_utils will not be built.")
  return()
endif()

# include_directories(${CMAKE_SOURCE_DIR}/core_utils)
add_library(zoom_utils STATIC
  zoom_utils.h
  zoom_utils.cpp

  zoom_window.h
  zoom_window.cpp

  scalable_label.h
  scalable_label.cpp)

set(FractalUtils_zoom_utils_headers
  zoom_utils.h
  zoom_window.h)

target_link_libraries(zoom_utils PUBLIC
  Qt6::Widgets
  core_utils)

target_compile_features(zoom_utils PUBLIC cxx_std_17)

add_library(fractal_utils::zoom_utils ALIAS zoom_utils)

# install libs
target_include_directories(zoom_utils
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  INTERFACE
  $<INSTALL_INTERFACE:include>)

if(NOT ${FractalUtils_no_install})
  # install the target and create export-set
  install(TARGETS zoom_utils
    EXPORT fractal_utils_zoom_utils-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/fractal_utils
  )

  # install header file
  install(FILES ${FractalUtils_zoom_utils_headers} DESTINATION include/fractal_utils)

  # generate and install export file
  install(EXPORT fractal_utils_zoom_utils-targets
    FILE fractal_utilszoom_utilsTargets.cmake
    NAMESPACE fractal_utils::
    DESTINATION lib/cmake/fractal_utils
  )
endif()

if(NOT ${FractalUtils_build_examples})
  return()
endif()

# example executable
add_executable(test_zoom_utils
  test_zoom_utils.cpp)

add_executable(test_zoom_window
  test_zoom_window.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(test_zoom_utils PRIVATE
  zoom_utils
  Qt6::Widgets
  core_utils
  OpenMP::OpenMP_CXX)

target_link_libraries(test_zoom_window PRIVATE
  zoom_utils
  Qt6::Widgets
  core_utils
  OpenMP::OpenMP_CXX)

if(${WIN32})
  find_program(FU_windeployqt NAME windeployqt PATHS ${CMAKE_PREFIX_PATH})

  if(NOT FU_windeployqt)
    message(WARNING "Fractal Utils failed to find windeployqt.")
  else()
    message(STATUS "windeployqt found at ${FU_windeployqt}")
    add_custom_target(FU_test-zoomer-deployqt
      COMMAND ${FU_windeployqt} test_zoom_utils.exe
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS test_zoom_utils)
  endif()
endif()

if(NOT ${FractalUtils_no_test})
  add_test(NAME FractalUtils_test_zoom_utils
    COMMAND test_zoom_utils
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()