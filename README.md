# gb-emu
Yet another gameboy emulator, written in C++

## Structure
- `src` Folder for all source code.
- `waf(.bat)` Waf command script for building code.
- `wscript` Actual build script used by waf.

## Building
Currently only msvc is supported. A recent python installation (2.X or 3.X) is
needed to run waf. Available commands are the standard waf commands
(`configure` `build`) plus the following:

- `dbuild` Same as `build` except that the debug variant is built.
- `msvs` Generate a visual studio solution in `.vs`. The solution uses the
         debug variant.