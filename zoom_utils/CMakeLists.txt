project(FractalUtils_zoom_utils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

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

include_directories(${CMAKE_SOURCE_DIR}/core_utils)

add_library(zoom_utils STATIC zoom_utils.h zoom_utils.cpp)

target_link_libraries(zoom_utils PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                         core_utils)
