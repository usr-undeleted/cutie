#!/bin/bash

## definitions
# compiler
compiler="clang"
# where the script is located
scriptdir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
scriptpath="$scriptpath/$(basename -- "${BASH_SOURCE[0]}")"
# where to search source dirs
sourcepath="$scriptdir/src/*.c"
# if there was an error
error=2
hadsuccess=0
# custom compile flags
extraflags=""
# the compiled command, use this to refer to the default command
defaultcompile="$compiler -I src/cutie-common.h -o bin/<program> src/<program>.c"

## intro
printf "\e[34;1;3m=== Interactive cutie compiling script! ===\e[0m\n\n"
printf "\e[1m"
cat $scriptdir/logo/cutie-ascii.txt
printf "\n\e[37;3mTip: type in program names as parameters for\ncommand invocation in order to compile \nthem without asking.\e[0m\n\n"
# latest commit
printf "\e[3;36mLatest git commit:\e[0m\n"
git -C $scriptdir show --summary HEAD | cat

## show all available programs and get binaries
printf "\n\e[3;36mAvailable programs (.c not included):\e[0m\n"
binaries=()
for file in $sourcepath; do
    printf "\e[1m$(basename $(basename "$file" src/) .c)\e[0m  "
    binaries+=("$(basename $(basename "$file" src/) .c)")
done
printf "\n\n"

## compile
compile() {
    local binary="$1"
    $compiler -I src/cutie-common.h -o $scriptdir/bin/$(basename "$binary" .c) $scriptdir/src/$binary.c $extraflags
    if [ $? -ne 0 ]; then
        printf "\e[31;1mFailed to compile $binary.\e[0m"
        if [ $hadsuccess -eq 1 ]; then
                error=1
        fi
        return
    fi
    printf "\e[32mdone compiling!\e[0m"
    hadsuccess=1
    if [ $error -ne 1 ]; then
        error=0
    fi
}

## if the user provided args, (try) to compile them. assumes they are in src/
if (( $# > 0 )); then
    printf "\e[3mEverything will be compiled to $scriptdir/bin/.\nPrograms specified are looked for in $scriptdir/src/, and the input should NOT have a '.c' extension.\e[0m\n"
    for arg; do
        printf "\r\e[33;3mCompiling '$arg'...\e[0m\e[K  "
        compile "$arg"
        printf "\n"
    done
    exit $error
fi

## ask the user what they want to compile
printf "\e[3;36mNow, pick which programs you'll compile. They'll all be compiled to $scriptdir/bin/.\e[0m"
if [ ! -d "$scriptdir/bin" ]; then
    mkdir $scriptdir/bin
fi

# if they want to compile everything at once
printf "\e[1m\n"
allofthem=0
while true; do
    read -n1 -p "Would you like to compile everything at once? (y/n): " allinput
    # handle newlines
    if [[ -z $allinput || $allinput == $'\n' ]]; then
        printf "\e[A"
    fi

    if [[ $allinput == [yY] ]]; then
        allofthem=1
        printf "\nCompiling everything!"
        break
    elif [[ $allinput == [nN] ]]; then
        break
    else
        printf "\r"
    fi
done
printf "\e[0m"

# if they want to add compile flags
printf "\e[1m\n"
while true; do
    read -n1 -p "Would you like to add custom compile flags? (y/n): " flaginput
    # handle newlines
    if [[ -z $flaginput || $flaginput == $'\n' ]]; then
        printf "\e[A"
    fi

    if [[ $flaginput == [yY] ]]; then
        printf "\n\e[37;3mCurrent base command: '$defaultcompile'\n\e[0;1m"
        read -p "Type in all the compile flags you'll use: " extraflags
        printf "\e[A"
        break
    elif [[ $flaginput == [nN] ]]; then
        break
    else
        printf "\r"
    fi
done
printf "\e[0m\n"

if [[ $allofthem == 1 ]]; then
    # dont ask
    for file in "${binaries[@]}"; do
        printf "\n"
        printf "\r\e[33;3mCompiling '$file'...\e[0m\e[K  "
        compile $file
    done
else
    # ask the user
    if [[ $extraflag != "" ]]; then
        printf "\n"
    fi

    for file in "${binaries[@]}"; do
        while true; do
            printf "\n"
            printf "Compile "
            read -n1 -p "'$(basename $(basename "$file" src/) .c)'? (y/n): " input
            if [[ $input == [yY] ]]; then
                printf "\r\e[33;3mCompiling '$file'...\e[0m\e[K  "
                compile $file
                break

            elif [[ $input == [nN] ]]; then
                printf "\r\e[31mCanceled compiling '$file'.\e[0m"
                break

            else
                true

            fi
        done
    done
fi

printf "\n\n"

## finished messages
# 1 for some mistakes, 2 for total failure
if [[ $error == 0 ]]; then
    printf "\e[36;1mFinished compiling all selected programs with no problem!\e[0m\n"
elif [[ $error == 1 ]]; then
    printf "\e[33;1mFinished compiling, but there were errors.\e[0m\n"
else
    printf "\e[31;1mDid not compile a single program.\e[0m\n"
fi
exit $error
