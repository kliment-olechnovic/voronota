cmake_minimum_required(VERSION 3.1)

project(Voronota-GL)

file(GLOB_RECURSE VORONOTA_GL_SRC "${PROJECT_SOURCE_DIR}/../expansion_gl/src/*.cpp" "${PROJECT_SOURCE_DIR}/../expansion_js/src/dependencies/*.cpp")
include_directories("${PROJECT_SOURCE_DIR}/../expansion_gl/src/dependencies" "${PROJECT_SOURCE_DIR}/../expansion_js/src/dependencies")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-gl ${VORONOTA_GL_SRC})

target_link_libraries(voronota-gl m GL GLEW glfw)

install(TARGETS voronota-gl RUNTIME DESTINATION bin)
