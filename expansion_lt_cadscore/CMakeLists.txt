cmake_minimum_required(VERSION 3.6)

project(Voronota-LT-CAD-score)

file(GLOB_RECURSE VORONOTA_LT_CADSCORE_SRC "${PROJECT_SOURCE_DIR}/../expansion_lt_cadscore/src/*.cpp")
include_directories("${PROJECT_SOURCE_DIR}/../expansion_lt/src/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(cadscore-lt ${VORONOTA_LT_CADSCORE_SRC})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cadscore-lt PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS cadscore-lt RUNTIME DESTINATION bin)

