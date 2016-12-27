cmake_minimum_required(VERSION 2.6)

project(Voronota)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.h)

set(CMAKE_BUILD_TYPE Release)

find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable(voronota ${sources} ${headers})

install(TARGETS voronota RUNTIME DESTINATION bin)
install(PROGRAMS voronota-cadscore voronota-contacts voronota-resources voronota-voromqa voronota-volumes DESTINATION bin)
