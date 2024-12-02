# Sudoku Solver

A modern Sudoku puzzle solver with a clean, dark-themed GUI built using Qt6 and C++17. The solver uses a Ternary Search Tree algorithm for efficient puzzle solving.

![Sudoku Solver Screenshot](https://i.ibb.co/rF3HpBb/IMG-6884.png)

## Features

- 🎨 Modern dark-themed user interface
- ✨ Real-time puzzle validation
- 🚨 Instant conflict highlighting
- ⚡️ Fast solving algorithm using Ternary Search Tree
- 🎮 Interactive grid with input validation
- 🎯 Example puzzle loading
- 🖥️ Cross-platform compatibility (macOS, Linux, Windows)

## Requirements

- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.16 or higher
- Qt6 (Core and Widgets components)
- Git (for version control)

## Installation

### macOS (using Homebrew)

```bash
# Install required dependencies
brew install cmake
brew install qt@6

# Clone and build
git clone https://github.com/Kandel-Bibas/Suduko.git
cd Suduko
mkdir build && cd build
cmake ..
make
# or on the main directory
cmake . && make && ./bin/SudokuSolver

#clean up
rm -rf build CMakeCache.txt CMakeFiles cmake_install.cmake Makefile bin
```

### Linux (Ubuntu/Debian)

```bash
# Install required dependencies
sudo apt update
sudo apt install cmake
sudo apt install qt6-base-dev

# Clone and build
git clone https://github.com/Kandel-Bibas/Suduko.git
cd Suduko
mkdir build && cd build
cmake ..
make
```

### Windows

1. Install [Qt6](https://www.qt.io/download) and [CMake](https://cmake.org/download/)
2. Add Qt and CMake to your PATH
3. Open Command Prompt and run:

```cmd
git clone https://github.com/Kandel-Bibas/Suduko.git
cd Suduko
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Usage

1. Launch the application
2. Enter numbers (1-9) directly into the grid cells
3. Use "Validate" to check for conflicts (highlighted in red)
4. Click "Solve" to solve the puzzle automatically
5. Use "Clear" to reset the grid
6. Try "Load Example" for a sample puzzle

## How It Works

The solver uses a Ternary Search Tree data structure to efficiently track and validate possible numbers for each cell. The algorithm:

1. Validates rows, columns, and 3x3 boxes in real-time
2. Uses backtracking with optimized candidate selection
3. Provides immediate visual feedback for conflicts
4. Ensures puzzle solvability before attempting solution

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Qt framework for the modern GUI components
- C++17 features for efficient memory management
- The Sudoku solving community for algorithm insights