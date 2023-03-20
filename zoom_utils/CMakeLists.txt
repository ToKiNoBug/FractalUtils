project(FractalUtils_zoom_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)

if(NOT ${QT_FOUND})
  message(WARNING "Qt6 Widgets not found. zoom_utils will not be built.")
  return()
endif()

# include_directories(${CMAKE_SOURCE_DIR}/core_utils)
add_library(zoom_utils STATIC
  zoom_utils.h
  zoom_utils.cpp
  scalable_label.h
  scalable_label.cpp)

target_link_libraries(zoom_utils PUBLIC
  Qt${QT_VERSION_MAJOR}::Widgets
  core_utils)

target_compile_features(zoom_utils PUBLIC cxx_std_17)

add_library(fractal_utils::zoom_utils ALIAS zoom_utils)

# install libs
target_include_directories(zoom_utils
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  INTERFACE
  $<INSTALL_INTERFACE:include>)

# install the target and create export-set
install(TARGETS zoom_utils
  EXPORT fractal_utils_zoom_utils-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include/fractal_utils
)

# install header file
install(FILES zoom_utils.h DESTINATION include/fractal_utils)

# generate and install export file
install(EXPORT fractal_utils_zoom_utils-targets
  FILE fractal_utilszoom_utilsTargets.cmake
  NAMESPACE fractal_utils::
  DESTINATION lib/cmake/fractal_utils
)

if(NOT ${FractalUtils_build_examples})
  return()
endif()

# example executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(test_zoom_utils
    MANUAL_FINALIZATION
    test_zoom_utils.cpp)
endif()

find_package(OpenMP REQUIRED)
target_link_libraries(test_zoom_utils PRIVATE
  zoom_utils
  Qt${QT_VERSION_MAJOR}::Widgets
  core_utils OpenMP::OpenMP_CXX)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(test_zoom_utils)
endif()
