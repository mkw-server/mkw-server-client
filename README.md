# MKW-Server Client

MKW-Server Client is the client-side component of MKW-Server. It is a fork of [MKW-SP](https://mkw-sp.com), an open-source mod for Mario Kart Wii, with most of its original features stripped out and replaced with MKW-Server-specific code.

## Building

You need:

- devkitPPC (with the DEVKITPPC environment variable set)
- ninja (samurai also works)
- protoc
- Python 3
- pyjson5 (if installing from pip, the package is `json5` NOT `pyjson5`)
- pyelftools
- itanium\_demangler
- protobuf (the Python package)

Compile the project by running `build.py`:

```bash
./build.py
```

The `out` directory will contain the generated binaries and assets.

## Contributing

If you are working on something please comment on the relevant issue (or open a new one if necessary).

The codebase uses C++, C and asm. C++ should be preferred for full function replacements and for any kind of complex logic. No assumption about the use of registers by C code should be made other than the ABI. If necessary asm wrappers can be employed to restore and backup volatile registers.

The codebase is automatically formatted using `clang-format` (20), this will be checked by CI and must be run before merge.

## Resources

- [Ghidra project](https://ghidra.decomp.dev/) (by far the most complete resource)

- [Pulsar](https://github.com/MelgMKW/Pulsar/) (strong header documentation)

- [MKW decompilation](https://github.com/doldecomp/mkw) (the most readable if it has what you need)

- [mkw-structures](https://github.com/SeekyCt/mkw-structures) (if you still haven't found)

- [Tockdom Wiki](http://wiki.tockdom.com/wiki/Main_Page) (file format documentation)
