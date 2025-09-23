#!/usr/bin/env bash

set -euo pipefail

OPTS=$(getopt -o 'sh?j::' -l 'silent,help,CC:,CXX:,LD:,CFLAGS:,CXXFLAGS:,CPPFLAGS:,LDFLAGS:,LIBS:' -n "$0" -- "$@")

j=1;
silent="false"

eval set -- "$OPTS"
unset OPTS

# parse args
while true; do
    case "$1" in
        '-s'|'--silent')
            silent="true"
            shift
        ;;
        '-h'|'--help')
            echo "Options:"
            echo " -h, --help           print help message"
            echo " -j, -j <num>         enable jobs/set max jobs"
            echo " --CC <var>           set compiler for C,         default '$(cat "build-defaults.json" | jq -r .CC)'"
            echo " --CXX <var>          set compiler for C++,       default '$(cat "build-defaults.json" | jq -r .CXX)'"
            echo " --LD <var>           set linker,                 default '$(cat "build-defaults.json" | jq -r .LD)'"
            echo " --CFLAGS <var>       set flags for C,            default '$(cat "build-defaults.json" | jq -r .CFLAGS)'"
            echo " --CXXFLAGS <var>     set flags for C++,          default '$(cat "build-defaults.json" | jq -r .CXXFLAGS)'"
            echo " --CPPFLAGS <var>     set flags for C and C++,    default '$(cat "build-defaults.json" | jq -r .CPPFLAGS)'"
            echo " --LDFLAGS <var>      set flags for linking,      default '$(cat "build-defaults.json" | jq -r .LDFLAGS)'"
            echo " --LIBS <var>         set libraries for linking,  default '$(cat "build-defaults.json" | jq -r .LIBS)'"
            exit 0
        ;;
        '--')
            shift
            break
        ;;
        '-j'|'--jobs')
            ISNUM='^[0-9]*$'
            if ! [[ $2 =~ $ISNUM ]] ; then
               echo "$0: $1 requires a number, '$2' passed";
               exit 1
            fi
            declare ${1//-/}="$2"
            shift
            shift
        ;;
        *)
            declare ${1//-/}="$2"
            shift
            shift
        ;;
    esac
done

if [ -z $j ]; then
    j=$(nproc --all)
fi;

if [ -z ${CC+x} ]; then
    CC=$(cat "build-defaults.json" | jq -r .CC);
fi;

if [ -z ${CXX+x} ]; then
    CXX=$(cat "build-defaults.json" | jq -r .CXX);
fi;

if [ -z ${LD+x} ]; then
    LD=$(cat "build-defaults.json" | jq -r .LD);
fi;

if [ -z ${CFLAGS+x} ]; then
    CFLAGS=$(cat "build-defaults.json" | jq -r .CFLAGS);
fi;

if [ -z ${CXXFLAGS+x} ]; then
    CXXFLAGS=$(cat "build-defaults.json" | jq -r .CXXFLAGS);
fi;

if [ -z ${CPPFLAGS+x} ]; then
    CPPFLAGS=$(cat "build-defaults.json" | jq -r .CPPFLAGS);
fi;

if [ -z ${LDFLAGS+x} ]; then
    LDFLAGS=$(cat "build-defaults.json" | jq -r .LDFLAGS);
fi;

if [ -z ${LIBS+x} ]; then
    LIBS=$(cat "build-defaults.json" | jq -r .LIBS);
fi;

function runjob
{
    EXECUTABLE=$1
    shift
    if [ "$silent" = "false" ]; then
        echo $EXECUTABLE "$@"
    fi;
    
    if [ $(jobs | wc -l | xargs) -ge "$j" ]; then
        wait -n
    fi;
    $EXECUTABLE "$@" &
}

shopt -s nullglob

function compile
{
    for file in $1/*.cpp; do
        runjob $CXX -c $CPPFLAGS $CXXFLAGS -I../include $file
    done
    
    for file in $1/*.c; do
        runjob $CC -c $CPPFLAGS $CFLAGS -I../include $file
    done
    
    for folder in $1/*/; do
        compile $folder
    done
}

mkdir -p build_tmp;
cd build_tmp;

#(rm *.o  1> /dev/null 2> /dev/null) || true

compile "../src"

wait

set +e

echo "$LD $LDFLAGS *.o $LIBS -o BitRot"
$LD $LDFLAGS *.o $LIBS -o ../BitRot

cd ..
rm -rf build_tmp
