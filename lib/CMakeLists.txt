cmake_minimum_required(VERSION 3.2)

project(cobs_lib)

add_library(cobs_lib cobs.cpp)

install(TARGETS cobs_lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES cobs.hpp
  DESTINATION include
)
