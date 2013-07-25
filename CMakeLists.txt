cmake_minimum_required(VERSION 2.6)

project(APOLLOTA)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.h)

set(CMAKE_BUILD_TYPE Release)

add_executable(apollota ${sources} ${headers})
