cmake_minimum_required(VERSION 3.0)
project(oop-ga-class04)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(SOURCE_FILES main.cpp scala.cpp)
add_executable(scala-sim ${SOURCE_FILES})
