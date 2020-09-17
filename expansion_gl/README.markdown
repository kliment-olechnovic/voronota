# Building from source code

## Requirements

Voronota-GL requires the following libraries: m GL GLEW glfw

## Using CMake

You can build using CMake for makefile generation.
Starting in the directory containing "CMakeLists.txt" file,
run the sequence of commands:

    cmake ./
    make

Alternatively, to keep files more organized, CMake can be run in a separate "build" directory:

    mkdir build
    cd build
    cmake ../
    make
    cp ./voronota-gl ../voronota-gl

## Using C++ compiler directly

For example, "voronota-gl" executable can be built from
the sources in "src" directory using GNU C++ compiler:

    g++ -std=c++14 -I"./src/expansion_js/src/dependencies" -I"./src/expansion_gl/src/dependencies" -O3 -o "./voronota-gl" $(find ./src/ -name '*.cpp') -lm -lGL -lGLEW -lglfw

