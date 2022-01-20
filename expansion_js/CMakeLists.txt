cmake_minimum_required(VERSION 3.1)

project(Voronota-JS)

file(GLOB_RECURSE VORONOTA_JS_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

include_directories("${PROJECT_SOURCE_DIR}/src/expansion_js/src/dependencies")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-js ${VORONOTA_JS_SRC})

install(TARGETS voronota-js RUNTIME DESTINATION bin)
install(PROGRAMS voronota-js-voromqa DESTINATION bin)
install(PROGRAMS voronota-js-membrane-voromqa DESTINATION bin)
install(PROGRAMS voronota-js-ifeatures-voromqa DESTINATION bin)
