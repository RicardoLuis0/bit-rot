mkdir -p build;
cd build;
set -euo pipefail

if [ -z ${CC+x} ]; then
    CC='gcc-12';
fi;

if [ -z ${CXX+x} ]; then
    CXX='g++-12';
fi;

if [ -z ${LD+x} ]; then
    LD='g++-12';
fi;

if [ -z ${CXXFLAGS+x} ]; then
    CXXFLAGS='-std=c++23 -Wno-literal-suffix';
fi;

if [ -z ${CFLAGS+x} ]; then
    CFLAGS='-std=c17';
fi;

if [ -z ${CPPFLAGS+x} ]; then
    CPPFLAGS='';
fi;

if [ -z ${LDFLAGS+x} ]; then
    LDFLAGS='-std=c++23';
fi;

if [ -z ${LIBS+x} ]; then
    LIBS='-lSDL2 -lGLEW -lGL -lGLU -lz -lbacktrace';
fi;

set -v

rm *.o

$CXX -c $CPPFLAGS $CXXFLAGS -I../include ../src/*.cpp ../src/*/*.cpp ../src/*/*/*/*.cpp
$CC -c $CPPFLAGS $CFLAGS -I../include ../src/*/*.c ../src/*/*/*.c
$LD $LDFLAGS *.o $LIBS -o BitRot
