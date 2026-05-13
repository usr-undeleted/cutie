# cutie Core UTIlity Environment
<p align="center">
  <img src="logo/cutie.png" alt="cutie" width="335">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/license-GPLv3-blue">
</p>

Undeleted's life long goal to write their own core utils, following their own design principles, all in C alongside assembly (once i actually learn it-).  

### Will strive for:
- Reusing as many utilities in the code as possible. Move commonly used functions to a common lib.
- Only type of fancy you'll see is colors. Maybe.
- Only using what you need.
- Keep It Simple, Stupid, I guess...
- Bin naming should have its name instantly obvious to the user. "create something", "scan something, a directory"
- If the shell can do it, don't bother.

## Current implementations:
- scan directories (scan); replace both ls and find, use --recursive|-r for find's functionality, otherwise, ls by default
- print dir path (printpath); replace both pwd and realpath
- printf wrapper (print)
- query search (query)
- file reading (show)
- install script (build.sh); enable usage with 'chmod +x build.sh'. Instructions on command output. Includes even custom compile flags!!

## Planned:
- file/dir removal (rm); applies to directories and regular files
- file/dir creation (create); create directories with "name/". should support recursion, with flag or not (create folder/file)
- timeout (timeout); :p

## Compiling:
- A script (build.sh) is available for usage to compile specific programs. It is interactive!
- To use it, run:
```
chmod +x build.sh
```
- That will make the script executable. Now, to execute it:
```
./build.sh
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
- Reuse utilities as much as possible; avoid adding new headers
- If a function is used in more than one bin, put it in a commom lib (src/cutiecommon.h)
- Vibe coding WILL be rejected. The reviewing WILL be done by humans ONLY. Using AI for coding assitance is acceptable, as long as it respects the rules set.
- Errors should follow this standard:
  - 1: Code issue, like failed malloc, etc.
  - 2+: User issue, like permissions, non-existent file, etc.
