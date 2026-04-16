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
- printf wrapper (print)
- query search (search)
- file reading (show)
- file/dir removal (rm); applies to directories and regular files
- file/dir creation (create); create directories with "name/". should support recursion, with flag or not (create folder/file)
- install script. Will let you pick compile flags at start (native march, debugging, output location, static linking, etc), and let the user pick what utils to compile.
