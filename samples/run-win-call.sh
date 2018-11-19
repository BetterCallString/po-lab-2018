#!/bin/bash

# Script to run all VSA on all test c-programs.
# Please specify the path to llvm and clang in the environment variables
# VSA_CLANG_PATH and VSA_LLVM_PATH.

while :; do
    case $1 in
        -t) flag1="TEST"
        ;;
        *) break
    esac
    shift
done

if [ "$flag1" = "TEST" ]; then
    EXE=llvm-vsa-tutorial.so
    PASS=vsatutorialpass
else
    EXE=llvm-vsa.so
    PASS=vsapass
fi

# if one argument passed: only analyze the passed program
if [ $# == 1 ] ; then
    ARRAY=($1)
else # run all
    ARRAY=($(ls -d call-*.c))
fi

# color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# if no folder is existent
mkdir -p build

# for all c-files...
for f in ${ARRAY[*]};
do
    # ... print file name
    echo "###############################################################################"
    echo $(pwd)/$f
    # ... clean up for old run
    rm -f build/$f.out
    rm -f build/$f.bc
    rm -f build/$f-opt.bc
    # ... compile
    $VSA_CLANG_PATH/bin/clang.exe -O0 -emit-llvm $f -Xclang -disable-O0-optnone -c -o build/$f.bc
    # ... run mem2reg optimization
    $VSA_LLVM_PATH/bin/opt.exe -mem2reg < build/$f.bc > build/$f-opt.bc
    # ... disassemble optimized file
    $VSA_LLVM_PATH/bin/llvm-dis.exe build/$f-opt.bc
    # ... run VSA
    $VSA_LLVM_PATH/bin/opt.exe -$PASS < build/$f-opt.bc > /dev/null 2> >(tee build/$f.out >&2)
    cp -n build/$f.out build/$f.ref
done

printf "\nTEST SUMMARY:\n"
for f in ${ARRAY[*]};
do
    # Find out to which file we are comparing the file
    if [ -e vectors/$f.txt ]
    then
        echo "Comparing against vector file vectors/$f.txt:"
        VECTOR_FILE="vectors/$f.txt"
    else
        echo "Comparing against the old output:"
        VECTOR_FILE="build/$f.ref"
    fi

    # Perform the actual comparison
    if cmp build/$f.out $VECTOR_FILE; then
        # ... success
        printf "Run: ${GREEN}$f${NC}\n"
    else
        # ... failure
        printf "Run: ${RED}$f${NC}\n"
    fi
done
printf "\n"
