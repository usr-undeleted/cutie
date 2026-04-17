# cutie Core UTIlity Environment
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

## Planned:
- print working dir (pwd); would be able to take in args to print the real path of selected stuff
- printf wrapper (print)
- query search (search)
- file reading (show)
- file/dir removal (rm); applies to directories and regular files
- file/dir creation (create); create directories with "name/". should support recursion, with flag or not (create folder/file)
- install script. Will let you pick compile flags at start (native march, debugging, output location, static linking, etc), and let the user pick what utils to compile.
- timeout (timeout); :p

### (temp) compiling:
- ALWAYS use clang; testing is done exclusively with clang
```
clang -I src/cutie-common.h -o bin/<name of bin> src/<name of bin>
```

## Code guidelines
- Small commenting, only when needed, as in:
  - Big section of code. Use comment to indicate its function
  - Unclear purpose of something
- Efficient, small as possible code
- Reuse utilities as much as possible; avoid adding new headers
- If a function is used in more than one bin, put it in a commom lib (src/cutiecommon.h)
- Vibe coding WILL be rejected. The reviewing WILL be done by humans ONLY. Using AI for coding assitance is acceptable, as long as it respects the rules set.
