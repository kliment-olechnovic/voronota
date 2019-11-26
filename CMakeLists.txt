cmake_minimum_required(VERSION 3.0)

project(Voronota)

file(GLOB VORONOTA_SRC "${PROJECT_SOURCE_DIR}/src/voronota.cpp" "${PROJECT_SOURCE_DIR}/src/modes/*.cpp")

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

add_executable(voronota ${VORONOTA_SRC})

install(TARGETS voronota RUNTIME DESTINATION bin)
install(PROGRAMS voronota-cadscore voronota-contacts voronota-resources voronota-voromqa voronota-volumes voronota-pocket voronota-membrane DESTINATION bin)
