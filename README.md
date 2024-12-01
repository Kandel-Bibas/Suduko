# Sudoku Solver

A modern Sudoku puzzle solver with GUI built using Qt6 and C++17. The solver uses a Ternary Search Tree algorithm for efficient puzzle solving.

## Features

- Modern, user-friendly GUI
- Real-time puzzle validation
- Conflict highlighting
- Fast solving algorithm
- Example puzzle loading
- Cross-platform compatibility

## Requirements

- C++17 compatible compiler
- CMake 3.16 or higher
- Qt6
- Git (for version control)

## Building from Source

### Unix-like Systems (Linux, macOS)

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/sudoku-solver.git
   cd sudoku-solver
   ```

2. Create build directory:
   ```bash
   mkdir build && cd build
   ```

3. Configure and build:
   ```bash
   cmake ..
   make
   ```

4. Run the application:
   ```bash
   ./bin/SudokuSolver
   ```

### Windows

1. Clone the repository:
   ```cmd
   git clone https://github.com/yourusername/sudoku-solver.git
   cd sudoku-solver
   ```

2. Create build directory:
   ```cmd
   mkdir build
   cd build
   ```

3. Configure and build:
   ```cmd
   cmake ..
   cmake --build . --config Release
   ```

4. Run the application:
   ```cmd
   .\bin\Release\SudokuSolver.exe
   ```

## Usage

1. Enter numbers (1-9) directly into the grid cells
2. Use the "Validate" button to check for conflicts
3. Click "Solve" to solve the puzzle
4. Use "Clear" to reset the grid
5. "Load Example" provides a sample puzzle to solve

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Qt framework for the GUI components
- Modern C++ features for efficient implementation
- Ternary Search Tree algorithm for puzzle solving 