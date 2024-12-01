#include "sudokugrid.h"
#include <QRegularExpressionValidator>
#include <set>

SudokuGrid::SudokuGrid(QWidget *parent) : QWidget(parent) {
    layout = new QGridLayout(this);
    layout->setSpacing(2);
    createGrid();
}

void SudokuGrid::createGrid() {
    cells.resize(9, std::vector<QLineEdit*>(9));
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            auto cell = new QLineEdit(this);
            cell->setAlignment(Qt::AlignCenter);
            cell->setMaxLength(1);
            cell->setFixedSize(45, 45);
            
            // Set input validation for numbers 1-9 only
            QRegularExpression rx("[1-9]");
            cell->setValidator(new QRegularExpressionValidator(rx, cell));
            
            styleCell(cell, i, j);
            
            connect(cell, &QLineEdit::textChanged, [this, cell]() {
                validateInput(cell);
                emit gridChanged();
                emit validityChanged(isValid());
            });
            
            cells[i][j] = cell;
            layout->addWidget(cell, i, j);
        }
    }
}

void SudokuGrid::styleCell(QLineEdit* cell, int row, int col) {
    QString borderStyle = "QLineEdit { "
                         "background-color: white; "
                         "font-size: 20px; "
                         "font-weight: bold; ";
    
    // Add borders based on 3x3 grid sections
    if (row % 3 == 0 && row != 0) {
        borderStyle += "border-top: 2px solid black; ";
    }
    if (col % 3 == 0 && col != 0) {
        borderStyle += "border-left: 2px solid black; ";
    }
    
    borderStyle += "border: 1px solid gray; }";
    borderStyle += "QLineEdit:focus { background-color: #f0f0f0; }";
    borderStyle += "QLineEdit[readOnly=\"true\"] { background-color: #e8e8e8; color: #666666; }";
    cell->setStyleSheet(borderStyle);
}

void SudokuGrid::validateInput(QLineEdit* cell) {
    QString text = cell->text();
    if (!text.isEmpty() && (text < "1" || text > "9")) {
        cell->clear();
    }
    clearHighlighting();
}

std::optional<int> SudokuGrid::getCellValue(int row, int col) const {
    QString value = cells[row][col]->text();
    if (value.isEmpty()) return std::nullopt;
    return value.toInt();
}

bool SudokuGrid::checkRowValid(int row) const {
    std::set<int> numbers;
    for (int col = 0; col < 9; ++col) {
        auto value = getCellValue(row, col);
        if (value && numbers.find(*value) != numbers.end()) {
            return false;
        }
        if (value) numbers.insert(*value);
    }
    return true;
}

bool SudokuGrid::checkColumnValid(int col) const {
    std::set<int> numbers;
    for (int row = 0; row < 9; ++row) {
        auto value = getCellValue(row, col);
        if (value && numbers.find(*value) != numbers.end()) {
            return false;
        }
        if (value) numbers.insert(*value);
    }
    return true;
}

bool SudokuGrid::checkBoxValid(int startRow, int startCol) const {
    std::set<int> numbers;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            auto value = getCellValue(startRow + i, startCol + j);
            if (value && numbers.find(*value) != numbers.end()) {
                return false;
            }
            if (value) numbers.insert(*value);
        }
    }
    return true;
}

bool SudokuGrid::isValid() const {
    // Check rows and columns
    for (int i = 0; i < 9; ++i) {
        if (!checkRowValid(i) || !checkColumnValid(i)) {
            return false;
        }
    }
    
    // Check 3x3 boxes
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!checkBoxValid(i, j)) {
                return false;
            }
        }
    }
    
    return true;
}

void SudokuGrid::highlightConflicts() {
    clearHighlighting();
    
    // Helper lambda to highlight conflicts
    auto highlightCell = [](QLineEdit* cell) {
        QString style = cell->styleSheet();
        style.replace("background-color: white", "background-color: #ffebee");
        cell->setStyleSheet(style);
    };
    
    // Check rows and columns
    for (int i = 0; i < 9; ++i) {
        if (!checkRowValid(i)) {
            for (int j = 0; j < 9; ++j) {
                if (!cells[i][j]->text().isEmpty()) {
                    highlightCell(cells[i][j]);
                }
            }
        }
        if (!checkColumnValid(i)) {
            for (int j = 0; j < 9; ++j) {
                if (!cells[j][i]->text().isEmpty()) {
                    highlightCell(cells[j][i]);
                }
            }
        }
    }
    
    // Check 3x3 boxes
    for (int boxRow = 0; boxRow < 9; boxRow += 3) {
        for (int boxCol = 0; boxCol < 9; boxCol += 3) {
            if (!checkBoxValid(boxRow, boxCol)) {
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        if (!cells[boxRow + i][boxCol + j]->text().isEmpty()) {
                            highlightCell(cells[boxRow + i][boxCol + j]);
                        }
                    }
                }
            }
        }
    }
}

void SudokuGrid::clearHighlighting() {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            styleCell(cells[i][j], i, j);
        }
    }
}

bool SudokuGrid::isFull() const {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (cells[i][j]->text().isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

void SudokuGrid::loadExample() {
    const std::vector<std::vector<int>> example = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };
    setGrid(example);
}

std::vector<std::vector<int>> SudokuGrid::getGrid() const {
    std::vector<std::vector<int>> grid(9, std::vector<int>(9, 0));
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QString value = cells[i][j]->text();
            grid[i][j] = value.isEmpty() ? 0 : value.toInt();
        }
    }
    return grid;
}

void SudokuGrid::setGrid(const std::vector<std::vector<int>>& grid) {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            int value = grid[i][j];
            cells[i][j]->setText(value == 0 ? "" : QString::number(value));
        }
    }
}

void SudokuGrid::clear() {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            cells[i][j]->clear();
        }
    }
    clearHighlighting();
} 