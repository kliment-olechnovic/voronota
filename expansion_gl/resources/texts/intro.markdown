# About Voronota-GL expansion

Voronota-GL is an expansion of [the core Voronota software](../index.html).
Voronota-GL is built on top of [the Voronota-JS expansion](../expansion_js/index.html).

Voronota-GL is a visual tool for the comprehensive interactive
analysis of macromolecular structures, including the Voronoi tesselation-based analysis.

Voronota-GL can be built to run as a standalone application, or as a WebAssembly application to run on a web page - [a working web version is available](./web/index.html).

## Usage of externally developed software

Voronota-GL relies on several externally developed software projects (big thanks to their authors):

* ImGui - [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)
* ImGuiColorTextEdit - [https://github.com/santaclose/ImGuiColorTextEdit](https://github.com/santaclose/ImGuiColorTextEdit)
* ImGuiFileDialog - [https://github.com/aiekick/ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
* GLFW - [https://www.glfw.org/](https://www.glfw.org/)
* GLM - [https://github.com/g-truc/glm](https://github.com/g-truc/glm)
* GLEW - [https://glew.sourceforge.net/](https://glew.sourceforge.net/)
* and all the externally developed software that is used in [the Voronota-JS expansion](../expansion_js/index.html)

# Getting the latest version

Download the latest archive from the official downloads page:
[https://github.com/kliment-olechnovic/voronota/releases](https://github.com/kliment-olechnovic/voronota/releases).

The archive contains the Voronota-GL software in the 'expansion_gl' subdirectory.
The 'voronota-gl' executable can be built from the provided source code.

# Building from source code on Linux

## Requirements

Voronota-GL requires the following dynamic libraries to link with: m GL GLEW glfw

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

    g++ -std=c++14 -I"../expansion_js/src/dependencies" -I"./src/dependencies" -O3 -o "./voronota-gl" $(find ../expansion_js/src/dependencies/ -name '*.cpp') $(find ./src/ -name '*.cpp' ) -lm -lGL -lGLEW -lglfw

# Building from source code on Mac OS

On Mac OS, "voronota-gl" executable can be built from
the sources in "src" directory using the following command:

    g++ -framework OpenGL -std=c++14 -I"../expansion_js/src/dependencies" -I"./src/dependencies" -O3 -o "./voronota-gl" $(find ../expansion_js/src/dependencies/ -name '*.cpp') $(find ./src/ -name '*.cpp' ) -lm -lGLEW -lglfw

