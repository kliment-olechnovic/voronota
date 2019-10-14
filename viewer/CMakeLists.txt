cmake_minimum_required(VERSION 3.0)

project(VoronotaViewer)

file(GLOB VORONOTA_VIEWER_SRC "${PROJECT_SOURCE_DIR}/src/voronota_viewer.cpp" "${PROJECT_SOURCE_DIR}/src/imgui/*.cpp")

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

add_executable(voronota-viewer ${VORONOTA_VIEWER_SRC})

target_link_libraries(voronota-viewer m GL GLEW glfw pthread)
