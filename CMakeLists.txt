cmake_minimum_required(VERSION 3.2)

include(ExternalProject)

ExternalProject_Add(cobs_lib
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib"
  CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install"
    "-DBUILD_SHARED_LIBS=on"
)

ExternalProject_Add(cobs_cli
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cli"
  CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install"
)

project(cobs)

