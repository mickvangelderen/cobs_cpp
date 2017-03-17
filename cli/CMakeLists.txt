cmake_minimum_required(VERSION 3.2)

project(cobs_cli)

add_executable(cobs main.cpp)

install(TARGETS cobs
  RUNTIME DESTINATION bin
)
