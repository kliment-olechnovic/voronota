cmake_minimum_required(VERSION 3.1)

project(Voronota-JS)

file(GLOB_RECURSE VORONOTA_JS_SRC "${PROJECT_SOURCE_DIR}/../expansion_js/src/*.cpp")
include_directories("${PROJECT_SOURCE_DIR}/../expansion_js/src/dependencies")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(voronota-js ${VORONOTA_JS_SRC})

install(TARGETS voronota-js RUNTIME DESTINATION bin)
install(PROGRAMS voronota-js-voromqa voronota-js-only-global-voromqa voronota-js-membrane-voromqa voronota-js-ifeatures-voromqa voronota-js-fast-iface-voromqa voronota-js-fast-iface-cadscore voronota-js-fast-iface-cadscore-matrix voronota-js-fast-iface-data-graph voronota-js-fast-iface-contacts voronota-js-voroif-gnn voronota-js-ligand-cadscore voronota-js-pdb-utensil-renumber-by-sequence voronota-js-pdb-utensil-detect-inter-structure-contacts voronota-js-pdb-utensil-split-to-models voronota-js-pdb-utensil-filter-atoms voronota-js-pdb-utensil-rename-chains voronota-js-pdb-utensil-print-sequence-from-structure voronota-js-pdb-utensil-download-structure DESTINATION bin)

