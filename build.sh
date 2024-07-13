mkdir -p build
cd build
rm *.o
g++-12 -c -std=c++23 -Wno-literal-suffix -I../include ../src/*.cpp ../src/*/*.cpp
gcc-12 -c -std=c17 -I../include ../src/*/*.c ../src/*/*/*.c
g++-12 -std=c++23 *.o -lSDL2 -lGLEW -lGL -lGLU -lz -lbacktrace -o BitRot
