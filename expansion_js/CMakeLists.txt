cmake_minimum_required(VERSION 3.1)

project(Voronota-JS)

file(GLOB_RECURSE VORONOTA_JS_SRC "${PROJECT_SOURCE_DIR}/../expansion_js/src/*.cpp")
include_directories("${PROJECT_SOURCE_DIR}/../expansion_js/src/dependencies")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-js ${VORONOTA_JS_SRC})

install(TARGETS voronota-js RUNTIME DESTINATION bin)
install(PROGRAMS voronota-js-voromqa voronota-js-membrane-voromqa voronota-js-ifeatures-voromqa voronota-js-fast-iface-voromqa voronota-js-fast-iface-cadscore voronota-js-fast-iface-cadscore-matrix voronota-js-fast-iface-data-graph DESTINATION bin)

