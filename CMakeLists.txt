cmake_minimum_required(VERSION 3.1)

project(Voronota)

file(GLOB VORONOTA_SRC "${PROJECT_SOURCE_DIR}/src/voronota.cpp" "${PROJECT_SOURCE_DIR}/src/modes/*.cpp")

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota ${VORONOTA_SRC})

install(TARGETS voronota RUNTIME DESTINATION bin)
install(PROGRAMS voronota-cadscore voronota-contacts voronota-resources voronota-voromqa voronota-volumes voronota-pocket voronota-membrane DESTINATION bin)
