#!/bin/bash

## definitions
# compiler
compiler="clang"
# where the script is located
scriptdir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")"/.. &> /dev/null && pwd)
scriptpath="$scriptpath/$(basename -- "${BASH_SOURCE[0]}")"
# custom compile flags
extraflags=""
# if they want to strip the binaries
stripbinary=0
# the compiled command, use this to refer to the default command
defaultcompile="$compiler -I $scriptdir/src/cutie-common.h -o $scriptdir/bin/sha $scriptdir/src/sha.c -lcrypto -Wno-deprecated-declarations -DSHA<algo>D"

printf "\e[34;1;3m=== Interactive cutie sha variant compiling script! ===\e[0m\n"
printf "\e[37;3mnote that we are using deprecated functions for sha; if you're willing to, replace the funcs with EVP ones.\e[0m\n"

## see if we have clang. if we dont, try gcc with a warning, and if even thats not present, exit
if ! command -v clang >/dev/null 2>&1
then
    printf "\e[33;1mWARNING! Clang is not present on this system. GCC will be used (if available), but keep in mind that any bugs caused by the use of a different compiler isn't cutie's fault.\e[0m\n\n"
    compiler="gcc"

    if ! command -v gcc >/dev/null 2>&1
    then
        printf "\e[31;1mGCC wasn't found. Cancelling.\e[0m\n"
        exit 1
    fi
fi

## if they want to strip the binaries
printf "\e[1m\n"
while true; do
    read -n1 -p "Would you like to strip the binaries? (y/N): " flaginput
    # handle newlines
    if [[ -z $flaginput || $flaginput == $'\n' ]]; then
        printf "\e[A"
    fi

    if [[ $flaginput == [yY] ]]; then
        stripbinary=1
        break
    elif [[ -z $input || $flaginput == [nN] ]]; then
        break
    else
        printf "\r"
    fi
done
printf "\e[0m\n"

## if they want to add compile flags
printf "\e[1m\n"
while true; do
    read -n1 -p "Would you like to add custom compile flags? (y/N): " flaginput
    # handle newlines
    if [[ -z $flaginput || $flaginput == $'\n' ]]; then
        printf "\e[A"
    fi

    if [[ $flaginput == [yY] ]]; then
        printf "\n\e[37;3mCurrent base command: '$defaultcompile'\n\e[0;1m"
        read -p "Type in all the compile flags you'll use: " extraflags
        printf "\e[A"
        break
    elif [[ -z $input || $flaginput == [nN] ]]; then
        break
    else
        printf "\r"
    fi
done
printf "\e[0m\n\n"

## ask the user on what algo to use
appended=""
algoarray=("1" "224" "256" "384" "512")
printf "\e[1mAvailable algorithms: "
for printed in "${algoarray[@]}"; do
    printf "$printed"
    if [ $printed != "512" ]; then printf " - "; fi
done
printf "\e[0m\n"
while true; do
    error=1
    read -p "Type in an available algorithm: " algoinput

    for compare in "${algoarray[@]}"; do
        if [ $compare == $algoinput ]; then
            error=0
            appended="$compare"
            break
        fi
    done

    if (( error == 1 )); then
        printf "\e[31mNon valid algo selected, try again.\e[0m\n\e[2A\e[K"
    else
        break
    fi
done

## finally, compile
printf "\e[33;3m%s%s\e[0m\e[K" "Compiling sha$algo$appended""sum...  "
# make bin if it hasnt been made
if [ ! -d "$scriptdir/bin" ]; then
    mkdir $scriptdir/bin
fi

$compiler -I $scriptdir/src/cutie-common.h -o $scriptdir/bin/sha$appended"sum" $scriptdir/src/sha.c -lcrypto -Wno-deprecated-declarations -DSHA$appended"D" $extraflags
if [ $? -ne 0 ]; then
    printf "\e[31;1mFailed to compile.\e[0m"
else
    printf "\e[32;1mDone compiling!\e[0m\n"
fi

printf "\e[33;3mStripping binary...\e[0m"
strip $scriptdir/bin/sha$appended"sum"
if [ $? -ne 0 ]; then
    printf " \e[31mfailed to strip $binary.\e[0m"
else
    printf " \e[32;1mstripped succesfully!\e[0m"
fi
printf "\n"
