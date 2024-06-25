cmake_minimum_required(VERSION 3.1)

project(Voronota-LT)

file(GLOB_RECURSE VORONOTA_LT_SRC "${PROJECT_SOURCE_DIR}/../expansion_lt/src/*.cpp")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-lt ${VORONOTA_LT_SRC})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(voronota-lt PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS voronota-lt RUNTIME DESTINATION bin)

