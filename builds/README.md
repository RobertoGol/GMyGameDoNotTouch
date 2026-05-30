# Build output

Configure CMake with a generator-specific subdirectory, for example:

```bat
cmake -S .. -B msvc
cmake --build msvc --config Debug
```

Run all commands from the repository root (`cmake -S . -B builds/msvc`).

This folder is gitignored.
