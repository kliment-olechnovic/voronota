cmake_minimum_required(VERSION 3.0)

project(Voronota)

file(GLOB voronota_src "${PROJECT_SOURCE_DIR}/src/voronota.cpp" "${PROJECT_SOURCE_DIR}/src/modes/*.cpp")

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -pedantic-errors -Wall -Werror -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")

add_executable(voronota ${voronota_src})

install(TARGETS voronota RUNTIME DESTINATION bin)
install(PROGRAMS voronota-cadscore voronota-contacts voronota-resources voronota-voromqa voronota-volumes DESTINATION bin)
