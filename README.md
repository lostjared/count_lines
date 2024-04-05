

# count_lines

## Description

This project consists of a C++ program designed to recursively traverse directories to find source files with specific extensions (`.cpp`, `.h`, `.hpp`, `.c`, `.cc`, `.cxx`, `.m`, `.mm`) and count the number of lines, distinguishing between code lines and blank lines. It's useful for developers or teams who want to analyze the size of their codebase or monitor its growth over time.

## Getting Started

### Dependencies

- C++ compiler (e.g., GCC, Clang)
- Linux/Unix environment (due to the use of specific headers like `<unistd.h>` and `<dirent.h>`)

### Compiling

To compile the program, navigate to the project directory and run:

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

### Running

After compilation, you can run the program by executing:

```bash
count_lines [directory_path]
```

If no directory path is provided, the program will default to the current directory.

## How It Works

The program takes a directory path as an argument (or defaults to the current directory) and recursively searches through it to find files with the specified extensions. It then counts the number of non-empty and empty lines in each file, providing a total count at the end, along with individual file details.

## Contributing

We welcome contributions to improve this project! Please feel free to fork the repository, make your changes, and submit a pull request. Whether it's adding more file extensions, optimizing the search algorithm, or fixing bugs, your help is appreciated.
