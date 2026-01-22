cmake_minimum_required(VERSION 3.6)

project(Voronota-LT-CAD-score)

file(GLOB_RECURSE VORONOTA_LT_CADSCORE_SRC "${PROJECT_SOURCE_DIR}/../expansion_lt_cadscore/src/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-lt-cadscore ${VORONOTA_LT_CADSCORE_SRC})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(voronota-lt-cadscore PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS voronota-lt-cadscore RUNTIME DESTINATION bin)

