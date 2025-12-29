# LFI3A Quick Start

LFI3A is a fun and educational programming language based on Moroccan Arabic (Darija). This guide will help you get started quickly.

## Installation

LFI3A is currently a C++ based interpreter. To build it, you need a C++ compiler (like GCC or Clang) and CMake.

```bash
mkdir build
cd build
cmake ..
make
```

## Running your first program

Create a file named `hi.lfi3a`:

```lfi3a
dir s = "Salam LFI3A!"
kteb(s)
```

Run it using the interpreter:

```bash
./lfi3a hi.lfi3a
```

## Next Steps

To dive deeper into the language, check out these resources:

- **[Main Readme](README.md)** - Project overview and entry point.
- **[Syntax Guide](syntax.md)** - Detailed keywords and control flow.
- **[Standard Library](standard_library.md)** - Built-in functions like `kteb`, `tul`, etc.
- **[Examples](../examples/)** - Browse sample code to see LFI3A in action.

## Community & Support

Join us in making programming more accessible to the Darija-speaking community!
