# cutie Core UTIlity Environment
<p align="center">
  <img src="logo/cutie.png" alt="cutie" width="335">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/license-GPLv3-blue">
</p>

Undeleted's life long goal to write their own core utils, following their own design principles, all in C alongside assembly (once i actually learn it-).  

## Will strive for:
- Reusing as many utilities in the code as possible. Move commonly used functions to a common lib.
- Only type of fancy you'll see is colors. *excluding scripts*
- Only using what you need.
- Keep It Simple, Stupid!!
- Bin naming should have its name instantly obvious to the user. "create something", "scan something, a directory"
- If the shell can do it, don't bother.

## Current implementations:
- scan directories (scan); replace both ls and find, use --recursive|-r for find's functionality, otherwise, ls by default
- print dir path (printpath); replace both pwd and realpath
- printf wrapper (printf)
- query search (query)
- file reading (show)
- file/dir creation (create); create directories with "name/". should support recursion, with flag or not (create folder/file)
- timeout (timeout); :p
- install script (build.sh); enable usage with 'chmod +x build.sh'. Instructions on command output. Includes even custom compile flags set by user!!
- shaXsum calculation (sha); includes all sha calculations supported by gnu's shaXsums, see the section under for scripts for context on the big walls of code.
- flood terminal (flood)
- fetching data (fetch); info like whoami and uname

## Scripts: 
- Scripts are provided under 'scripts/'. 
1. 'build.sh' is the script used for compiling, see the section for compiling.
2. 'shaXsum.sh' takes the sha.c source file, and limits its algorithm capabilities to a single algorithm, making for a smaller binary.
3. (more to be added, see plans)

## Planned:
- file/dir removal (rm); applies to directories and regular files
- move/rename files (mv or move, ive got to decide)
- create links like syms or hardlinks (link)
- modify permissions on a file (chmod or something else)
- copy files (copy)
- format input (format):
probally the most complicated here, in terms of how many things its gonna include.  
wc (taking input and showing its characteristics), head (read N first lines of input), tail (read N last lines of input), sort + rev (sort lines by characteristic), fmt (format lines and paragraphs), uniq (deduplicate adjacent lines), appending something to input (EOF, b4 or after a space, start of line, end or start of input, etc), replacing or removing parts of input, etc. are all gonna be included. *sorry to whoever would read the --help..!*  
flag management is gonna be like, for example:  
-s (sort lines), followed immediately by a letter like r (reverse) or a (alphabetical). Stackable, but executed in order, so -sra would first reverse order, then sort in alphabetical.  
-R (full reverse), not followed by anything, would reverse everything, char by char.  
note that the flags here arent gonna be stackable, so they each will have to be separated.  
arguments that expect input would be, like (f is the flag starter, X expects 1 arg, Y expects 2):
"-fXY x y y"  
note that only stdin shall be provided, because mixing I/O when, again, the shell can do it, is just dumb...  
- Optional QOF scripts that would do extend or delimit the functionalities of specific programs, the planned ones being:
1. If a program here is simply gnu's equivalent, compile it with gnu's naming. 

## Compiling:
- A script (build.sh) is available in scripts/ for usage to compile specific programs. It is interactive!
- To use it, run:
```
chmod +x scripts/build.sh
```
- That will make the script executable. Now, to execute it:
```
./scripts/build.sh
```
### Incase you need to compile manually:
- ALWAYS use clang; testing is done exclusively with clang
```
clang -I src/cutie-common.h -o bin/<program> src/<program>.c
```

## Code guidelines
- Small commenting, only when needed, as in:
  - Big section of code. Use comment to indicate its function
  - Unclear purpose of something
- Efficient, small as possible code
- Focus on good practice ALWAYS
- Reuse utilities as much as possible; avoid adding new headers
- If a function is used in more than one bin, put it in a commom lib (src/cutiecommon.h)
- Vibe coding WILL be rejected. The reviewing WILL be done by humans ONLY. Using AI for coding assistance is acceptable, as long as it respects the rules set.
- Errors should follow this standard:
  - 1: Code issue, like failed malloc, etc.
  - 2: User issue, like permissions, non-existent file, etc.
  - 3: If you can, use perror, if not, use fprintf on stderr.
### Specific design decisions
- Anything starting with - is a flag, with the only exceptions being something like create. Note that (with exceptions mentioned forwards) this means that no matter where you place a flag in the invocation, it should work.
- Stdin is represented by a '-'.
- Unless the program shouldn't due to its own intricacies (like a flag requiring an argument), use labelFlags(). Programs that only need --help and a special flag shouldn't use labelFlags(), but use the implementantions in things like printpath or printf.
- Flags that take in arguments need to ALWAYS be separate from other flags.
- If a program needs a compiler flag library to work, the compiling flag should only be used in it (build.sh has an example, for sha).
- The help function has to ALWAYS be named helpMenu(). Copy it over from other programs and make it consistent.
- You can 100% have a bit of fun by defining macros that would experimentally change how a binary would work, as long as that customization doesn't weigh in to the default compile size
