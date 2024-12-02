#include "sudokugrid.h"
#include <QRegularExpressionValidator>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTime>
#include <QVBoxLayout>
#include <QFrame>
#include <set>

SudokuGrid::SudokuGrid(QWidget *parent)
    : QWidget(parent)
    , layout(nullptr)
    , hintTimer(nullptr)
    , hintsEnabled(false)
    , isDarkTheme(false)
    , notesMode(false)
    , currentCell(nullptr)
    , solver(std::vector<std::vector<int>>(9, std::vector<int>(9, 0)))
{
    // Set up saves directory
    savesDirectory = QDir::homePath() + "/.sudoku_saves";
    ensureSavesDirectoryExists();
    
    createGrid();
}

void SudokuGrid::ensureSavesDirectoryExists() {
    QDir dir(savesDirectory);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

bool SudokuGrid::savePuzzleToFile(const QString& filename) {
    QString actualFilename = filename;
    if (actualFilename.isEmpty()) {
        // Generate filename based on current date/time
        actualFilename = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".sudoku";
    }
    
    if (!actualFilename.endsWith(".sudoku")) {
        actualFilename += ".sudoku";
    }
    
    QString fullPath = savesDirectory + "/" + actualFilename;
    QFile file(fullPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Save current state
    out << "# Sudoku puzzle saved on " << QDateTime::currentDateTime().toString() << "\n";
    out << "# Format: Each line represents a row, empty cells are 0\n\n";
    
    // Save grid state
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QString text = cells[i][j]->text();
            out << (text.isEmpty() ? "0" : text);
            if (j < 8) out << " ";
        }
        out << "\n";
    }
    
    // Save notes if any exist
    if (!notes.empty()) {
        out << "\n# Notes:\n";
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                auto it = notes.find(cells[i][j]);
                if (it != notes.end() && !it->second.empty()) {
                    out << i << " " << j << ":";
                    for (int note : it->second) {
                        out << " " << note;
                    }
                    out << "\n";
                }
            }
        }
    }
    
    file.close();
    emit puzzleSaved(actualFilename);
    return true;
}

bool SudokuGrid::loadPuzzleFromFile(const QString& filename) {
    QString fullPath = savesDirectory + "/" + filename;
    QFile file(fullPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    
    // Clear current state
    clear();
    notes.clear();
    
    // Skip comments
    QString line;
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        break;
    }
    
    // Read grid state
    int row = 0;
    do {
        if (line.isEmpty() || line.startsWith("#")) break;
        
        QStringList values = line.split(" ", Qt::SkipEmptyParts);
        if (values.size() != 9) continue;
        
        for (int col = 0; col < 9; ++col) {
            int value = values[col].toInt();
            if (value > 0 && value <= 9) {
                cells[row][col]->setText(QString::number(value));
            }
        }
        
        row++;
        if (row >= 9) break;
        
    } while (!in.atEnd() && (line = in.readLine().trimmed(), true));
    
    // Read notes if they exist
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        // Parse note line: "row col: note1 note2 ..."
        QStringList parts = line.split(":", Qt::SkipEmptyParts);
        if (parts.size() != 2) continue;
        
        QStringList pos = parts[0].trimmed().split(" ", Qt::SkipEmptyParts);
        if (pos.size() != 2) continue;
        
        int noteRow = pos[0].toInt();
        int noteCol = pos[1].toInt();
        if (noteRow < 0 || noteRow >= 9 || noteCol < 0 || noteCol >= 9) continue;
        
        QStringList noteValues = parts[1].trimmed().split(" ", Qt::SkipEmptyParts);
        std::vector<int> cellNotes;
        for (const QString& note : noteValues) {
            int value = note.toInt();
            if (value > 0 && value <= 9) {
                cellNotes.push_back(value);
            }
        }
        
        if (!cellNotes.empty()) {
            notes[cells[noteRow][noteCol]] = cellNotes;
            updateCellNotes(cells[noteRow][noteCol]);
        }
    }
    
    file.close();
    emit puzzleLoaded(filename);
    return true;
}

QStringList SudokuGrid::getSavedPuzzleFiles() const {
    QDir dir(savesDirectory);
    return dir.entryList(QStringList() << "*.sudoku", QDir::Files, QDir::Time);
}

void SudokuGrid::createGrid() {
    layout = new QGridLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(20, 20, 20, 20);
    
    cells.resize(9, std::vector<QLineEdit*>(9));
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            cells[i][j] = new QLineEdit(this);
            cells[i][j]->setAlignment(Qt::AlignCenter);
            cells[i][j]->setMaxLength(1);
            cells[i][j]->setFixedSize(65, 65);
            cells[i][j]->setFont(QFont("SF Pro Display", 28, QFont::DemiBold));
            
            // Set validator for numbers 1-9 only
            QRegularExpressionValidator* validator = new QRegularExpressionValidator(QRegularExpression("[1-9]"), cells[i][j]);
            cells[i][j]->setValidator(validator);
            
            // Connect signals for real-time validation
            connect(cells[i][j], &QLineEdit::textChanged, this, [this, cell = cells[i][j]]() {
                validateInput(cell);
            });
            
            connect(cells[i][j], &QLineEdit::textEdited, this, [this]() {
                emit moveAdded();
            });
            
            // Handle cell focus
            connect(cells[i][j], &QLineEdit::cursorPositionChanged, this, [this, cell = cells[i][j]]() {
                if (cell->hasFocus()) {
                    selectCell(cell);
                }
            });
            
            // Handle mouse click
            cells[i][j]->installEventFilter(this);
            
            styleCell(cells[i][j], i, j);
            layout->addWidget(cells[i][j], i, j);
        }
    }
    
    applyTheme(false); // Default to light theme
}

bool SudokuGrid::eventFilter(QObject* obj, QEvent* event) {
    QLineEdit* cell = qobject_cast<QLineEdit*>(obj);
    if (cell && event->type() == QEvent::MouseButtonPress) {
        selectCell(cell);
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

void SudokuGrid::selectCell(QLineEdit* cell) {
    if (!cell || cell == currentCell) return;
    
    // Clear previous selection
    if (currentCell) {
        styleCell(currentCell, -1, -1);
    }
    
    // Update current cell
    currentCell = cell;
    currentCell->setFocus();
    
    cell->setStyleSheet(cell->styleSheet() + QString(R"(
        QLineEdit {
            background-color: %1;
            border: 2px solid #0984e3;
        }
    )").arg(isDarkTheme ? "#485460" : "#f5f6fa"));
    
    emit cellSelected(cell);
}

void SudokuGrid::styleCell(QLineEdit* cell, int row, int col) {
    if (!cell) return;
    
    // Calculate borders
    bool isTopEdge = row % 3 == 0;
    bool isBottomEdge = row % 3 == 2;
    bool isLeftEdge = col % 3 == 0;
    bool isRightEdge = col % 3 == 2;
    
    QString borderStyle = QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            font-size: 28px;
            font-weight: 500;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 2px;
            margin: 0px;
            %4
            %5
            %6
            %7
        }
        QLineEdit:hover {
            background-color: %8;
        }
        QLineEdit:focus {
            border: 2px solid #0984e3;
            background-color: %9;
        }
        QLineEdit::placeholder {
            color: %10;
            font-size: 14px;
        }
    )")
    .arg(isDarkTheme ? "#2d3436" : "#ffffff")
    .arg(isDarkTheme ? "#ffffff" : "#2d3436")
    .arg(isDarkTheme ? "#485460" : "#dfe6e9")
    .arg(isTopEdge ? "border-top: 3px solid " + QString(isDarkTheme ? "#485460" : "#636e72") + ";" : "")
    .arg(isBottomEdge ? "border-bottom: 3px solid " + QString(isDarkTheme ? "#485460" : "#636e72") + ";" : "")
    .arg(isLeftEdge ? "border-left: 3px solid " + QString(isDarkTheme ? "#485460" : "#636e72") + ";" : "")
    .arg(isRightEdge ? "border-right: 3px solid " + QString(isDarkTheme ? "#485460" : "#636e72") + ";" : "")
    .arg(isDarkTheme ? "#485460" : "#f5f6fa")
    .arg(isDarkTheme ? "#485460" : "#f5f6fa")
    .arg(isDarkTheme ? "#a4b0be" : "#636e72");
    
    cell->setStyleSheet(borderStyle);
}

void SudokuGrid::validateInput(QLineEdit* cell) {
    if (!cell) return;
    
    QString text = cell->text().trimmed();
    bool hasGridChanged = false;
    
    if (text.isEmpty()) {
        cell->clear();
        hasGridChanged = true;
    } else {
        bool ok;
        int value = text.toInt(&ok);
        if (ok && value >= 1 && value <= 9) {
            // Store the current state before validation
            auto currentGrid = getGrid();
            cell->setText(QString::number(value));
            
            // Check if the move creates any conflicts
            if (!isValid()) {
                emit mistakeAdded();  // Signal that a mistake was made
                animateCell(cell, "#ff6b6b");  // Visual feedback for mistake
            } else {
                // Award points for correct moves
                bool isSolvable = solver.solve();
                solver.setGrid(currentGrid);  // Reset solver's grid
                if (isSolvable) {
                    emit scoreIncreased(10);  // Award 10 points for valid move
                    if (isFull() && isValid()) {
                        emit scoreIncreased(100);  // Bonus for completing puzzle
                    }
                }
            }
            hasGridChanged = true;
        } else {
            cell->clear();
            hasGridChanged = true;
        }
    }
    
    if (hasGridChanged) {
        // Check validity and highlight conflicts
        bool isValidNow = isValid();
        emit validityChanged(isValidNow);
        
        if (!isValidNow) {
            highlightConflicts();
        } else {
            clearHighlighting();
        }
        
        emit gridChanged();
        checkCompletion();
    }
}

std::optional<int> SudokuGrid::getCellValue(int row, int col) const {
    QString value = cells[row][col]->text();
    if (value.isEmpty()) return std::nullopt;
    return value.toInt();
}

bool SudokuGrid::checkRowValid(int row) const {
    std::vector<bool> used(10, false);
    for (int col = 0; col < 9; ++col) {
        QString text = cells[row][col]->text();
        if (!text.isEmpty()) {
            int num = text.toInt();
            if (used[num]) return false;
            used[num] = true;
        }
    }
    return true;
}

bool SudokuGrid::checkColumnValid(int col) const {
    std::vector<bool> used(10, false);
    for (int row = 0; row < 9; ++row) {
        QString text = cells[row][col]->text();
        if (!text.isEmpty()) {
            int num = text.toInt();
            if (used[num]) return false;
            used[num] = true;
        }
    }
    return true;
}

bool SudokuGrid::checkBoxValid(int startRow, int startCol) const {
    std::vector<bool> used(10, false);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            QString text = cells[startRow + i][startCol + j]->text();
            if (!text.isEmpty()) {
                int num = text.toInt();
                if (used[num]) return false;
                used[num] = true;
            }
        }
    }
    return true;
}

bool SudokuGrid::isValid() const {
    // Check all rows
    for (int i = 0; i < 9; ++i) {
        if (!checkRowValid(i)) return false;
    }
    
    // Check all columns
    for (int i = 0; i < 9; ++i) {
        if (!checkColumnValid(i)) return false;
    }
    
    // Check all 3x3 boxes
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!checkBoxValid(i, j)) return false;
        }
    }
    
    return true;
}

void SudokuGrid::highlightConflicts() {
    // First clear any existing highlighting
    clearHighlighting();
    
    // Track all cells that need highlighting
    std::set<QLineEdit*> conflictCells;
    
    // Check rows
    for (int row = 0; row < 9; ++row) {
        std::map<int, std::vector<QLineEdit*>> numbers;
        for (int col = 0; col < 9; ++col) {
            QString text = cells[row][col]->text();
            if (!text.isEmpty()) {
                numbers[text.toInt()].push_back(cells[row][col]);
            }
        }
        for (const auto& pair : numbers) {
            if (pair.second.size() > 1) {
                for (QLineEdit* cell : pair.second) {
                    conflictCells.insert(cell);
                }
            }
        }
    }
    
    // Check columns
    for (int col = 0; col < 9; ++col) {
        std::map<int, std::vector<QLineEdit*>> numbers;
        for (int row = 0; row < 9; ++row) {
            QString text = cells[row][col]->text();
            if (!text.isEmpty()) {
                numbers[text.toInt()].push_back(cells[row][col]);
            }
        }
        for (const auto& pair : numbers) {
            if (pair.second.size() > 1) {
                for (QLineEdit* cell : pair.second) {
                    conflictCells.insert(cell);
                }
            }
        }
    }
    
    // Check 3x3 boxes
    for (int boxRow = 0; boxRow < 9; boxRow += 3) {
        for (int boxCol = 0; boxCol < 9; boxCol += 3) {
            std::map<int, std::vector<QLineEdit*>> numbers;
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    QString text = cells[boxRow + i][boxCol + j]->text();
                    if (!text.isEmpty()) {
                        numbers[text.toInt()].push_back(cells[boxRow + i][boxCol + j]);
                    }
                }
            }
            for (const auto& pair : numbers) {
                if (pair.second.size() > 1) {
                    for (QLineEdit* cell : pair.second) {
                        conflictCells.insert(cell);
                    }
                }
            }
        }
    }
    
    // Highlight all conflict cells
    for (QLineEdit* cell : conflictCells) {
        QString currentStyle = cell->styleSheet();
        QString newStyle = currentStyle + QString(R"(
            QLineEdit {
                background-color: #ff6b6b;
                color: white;
                border: 2px solid #ff4757;
            }
        )");
        cell->setStyleSheet(newStyle);
    }
}

void SudokuGrid::clearHighlighting() {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            styleCell(cells[i][j], i, j);
            if (cells[i][j] == currentCell) {
                cells[i][j]->setStyleSheet(cells[i][j]->styleSheet() + QString(R"(
                    QLineEdit {
                        background-color: %1;
                        border: 2px solid #0984e3;
                    }
                )").arg(isDarkTheme ? "#485460" : "#f5f6fa"));
            }
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
    bool wasValid = isValid();
    bool hasGridChanged = false;
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QString newValue = grid[i][j] > 0 ? QString::number(grid[i][j]) : "";
            if (cells[i][j]->text() != newValue) {
                cells[i][j]->setText(newValue);
                hasGridChanged = true;
            }
        }
    }
    
    if (hasGridChanged) {
        bool isValidNow = isValid();
        if (wasValid != isValidNow) {
            emit validityChanged(isValidNow);
        }
        emit gridChanged();
        checkCompletion();
    }
}

void SudokuGrid::clear() {
    bool wasValid = isValid();
    bool hasGridChanged = false;
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (!cells[i][j]->text().isEmpty()) {
                cells[i][j]->clear();
                hasGridChanged = true;
            }
        }
    }
    
    if (hasGridChanged) {
        bool isValidNow = isValid();
        if (wasValid != isValidNow) {
            emit validityChanged(isValidNow);
        }
        emit gridChanged();
    }
    
    notes.clear();
    clearHighlighting();
}

void SudokuGrid::applyTheme(bool isDark) {
    isDarkTheme = isDark;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            styleCell(cells[i][j], i, j);
        }
    }
}

void SudokuGrid::setHints(bool enabled) {
    hintsEnabled = enabled;
    if (!enabled && hintTimer) {
        hintTimer->stop();
    }
}

void SudokuGrid::undoMove() {
    if (undoStack.empty()) return;
    
    redoStack.push_back(getCurrentState());
    SudokuGrid::GridState prevState = undoStack.back();
    undoStack.pop_back();
    applyState(prevState);
    emit gridChanged();
}

void SudokuGrid::redoMove() {
    if (redoStack.empty()) return;
    
    undoStack.push_back(getCurrentState());
    SudokuGrid::GridState nextState = redoStack.back();
    redoStack.pop_back();
    applyState(nextState);
    emit gridChanged();
}

QString SudokuGrid::exportToString() const {
    QString result;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QString value = cells[i][j]->text();
            result += value.isEmpty() ? "0" : value;
        }
    }
    return result;
}

bool SudokuGrid::importFromString(const QString& data) {
    if (data.length() != 81) return false;
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QChar value = data[i * 9 + j];
            if (!value.isDigit()) return false;
            cells[i][j]->setText(value == '0' ? "" : QString(value));
        }
    }
    
    clearHighlighting();
    emit gridChanged();
    return true;
}

SudokuGrid::GridState SudokuGrid::getCurrentState() const {
    SudokuGrid::GridState state;
    state.values = getGrid();
    state.moveCount = undoStack.size();
    return state;
}

void SudokuGrid::applyState(const SudokuGrid::GridState& state) {
    setGrid(state.values);
    clearHighlighting();
}

void SudokuGrid::pushState() {
    undoStack.push_back(getCurrentState());
    redoStack.clear();
    emit moveAdded();
}

void SudokuGrid::animateCell(QLineEdit* cell, const QString& color) {
    if (!cell) return;
    
    QPropertyAnimation* animation = new QPropertyAnimation(cell, "styleSheet");
    animation->setDuration(500);
    
    QString originalStyle = cell->styleSheet();
    QString highlightStyle = originalStyle + QString(R"(
        QLineEdit {
            background-color: %1;
            border: 2px solid %1;
        }
    )").arg(color);
    
    animation->setStartValue(originalStyle);
    animation->setEndValue(highlightStyle);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Reset style after animation
    QTimer::singleShot(750, [cell, originalStyle]() {
        cell->setStyleSheet(originalStyle);
    });
}

void SudokuGrid::showPossibleValues(int row, int col) {
    if (!hintsEnabled || !cells[row][col]->text().isEmpty()) return;
    
    std::set<int> used;
    
    // Check row
    for (int j = 0; j < 9; ++j) {
        auto value = getCellValue(row, j);
        if (value) used.insert(*value);
    }
    
    // Check column
    for (int i = 0; i < 9; ++i) {
        auto value = getCellValue(i, col);
        if (value) used.insert(*value);
    }
    
    // Check 3x3 box
    int boxRow = (row / 3) * 3;
    int boxCol = (col / 3) * 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            auto value = getCellValue(boxRow + i, boxCol + j);
            if (value) used.insert(*value);
        }
    }
    
    // Show possible values with subtle highlighting
    QString hintColor = isDarkTheme ? "#00b894" : "#0984e3";
    for (int num = 1; num <= 9; ++num) {
        if (used.find(num) == used.end()) {
            QLineEdit* hintCell = cells[row][col];
            QString currentStyle = hintCell->styleSheet();
            hintCell->setStyleSheet(currentStyle + QString(
                "QLineEdit {"
                "   border: 2px dashed %1;"
                "}"
            ).arg(hintColor));
            
            QTimer::singleShot(1500, [hintCell, currentStyle]() {
                hintCell->setStyleSheet(currentStyle);
            });
        }
    }
}

void SudokuGrid::checkCompletion() {
    if (!isFull() || !isValid()) {
        return;
    }
    
    // Get current grid state
    GridState state = getCurrentState();
    
    // Check if all cells are filled correctly
    bool isComplete = true;
    for (int i = 0; i < 9 && isComplete; ++i) {
        for (int j = 0; j < 9 && isComplete; ++j) {
            if (!cells[i][j]->text().isEmpty()) {
                int value = cells[i][j]->text().toInt();
                if (value < 1 || value > 9) {
                    isComplete = false;
                }
            } else {
                isComplete = false;
            }
        }
    }
    
    if (isComplete) {
        // Calculate time taken (for now just emit 0)
        emit puzzleSolved(0);
    }
}

void SudokuGrid::toggleNote(QLineEdit* cell, int number) {
    if (!cell || number < 1 || number > 9) return;
    
    auto it = notes.find(cell);
    if (it == notes.end()) {
        notes[cell] = std::vector<int>{number};
    } else {
        auto& cellNotes = it->second;
        auto numIt = std::find(cellNotes.begin(), cellNotes.end(), number);
        if (numIt == cellNotes.end()) {
            cellNotes.push_back(number);
            std::sort(cellNotes.begin(), cellNotes.end());
        } else {
            cellNotes.erase(numIt);
        }
    }
    
    updateCellNotes(cell);
}

void SudokuGrid::updateCellNotes(QLineEdit* cell) {
    if (!cell) return;
    
    auto it = notes.find(cell);
    if (it == notes.end() || it->second.empty()) {
        cell->setPlaceholderText("");
        return;
    }
    
    QString noteText;
    for (int num : it->second) {
        if (!noteText.isEmpty()) noteText += " ";
        noteText += QString::number(num);
    }
    
    cell->setPlaceholderText(noteText);
    cell->setStyleSheet(cell->styleSheet() + QString(R"(
        QLineEdit::placeholder {
            color: %1;
            font-size: 12px;
        }
    )").arg(isDarkTheme ? "#a4b0be" : "#636e72"));
}

void SudokuGrid::setNotesMode(bool enabled) {
    notesMode = enabled;
    
    // Update cursor for visual feedback
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            cells[i][j]->setCursor(enabled ? Qt::CrossCursor : Qt::IBeamCursor);
        }
    }
}

bool SudokuGrid::isNotesMode() const {
    return notesMode;
}

void SudokuGrid::showHint() {
    // Get current grid state
    std::vector<std::vector<int>> currentGrid = getGrid();
    solver.setGrid(currentGrid);
    
    if (!solver.solve()) {
        return;  // No solution exists
    }
    
    std::vector<std::vector<int>> solution = solver.getGrid();
    std::vector<std::pair<int, int>> emptyCells;
    
    // Find all empty cells
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (cells[i][j]->text().isEmpty()) {
                emptyCells.push_back({i, j});
            }
        }
    }
    
    if (emptyCells.empty()) return;
    
    // Choose a random empty cell
    int index = static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(emptyCells.size())));
    auto [row, col] = emptyCells[index];
    
    // Show the hint with animation
    QLineEdit* cell = cells[row][col];
    cell->setText(QString::number(solution[row][col]));
    
    animateCell(cell, "#0984e3");
}

void SudokuGrid::newGame(const QString& difficulty) {
    // Clear current grid
    clear();
    notes.clear();
    
    // Define puzzles for each difficulty
    const std::map<QString, std::vector<QString>> puzzles = {
        {"Easy", {
            "530070000600195000098000060800060003400803001700020006060000280000419005000080079",
            "170000006000061000004000700060004003080070050500800070007000400000150000200000098",
            "200080300060070084030500209000105408000000000402706000301007040720040060004010003",
            "000000657702400100350006000200000740000052000064000008000100503007009204928000000",
            "020000000000600003074080000000003002080040010600500000000010780500009000000000040",
            "100007090030020008009600500005300900010080002600004000300000010040000007007000300",
            "000000012000000003002300400001800005060070800000009000008500000900040500470000090",
            "000200000000060700700000009800000040010000050040000003200000004003010000000008000",
            "000000657702400100350006000200000740000052000064000008000100503007009204928000000"
        }},
        {"Medium", {
            "009000400200009000087002090030070502000000000704050060070200140000800007006000800",
            "020000000000600003074080000000003002080040010600500000000010780500009000000000040",
            "000000907000420180000705026100904000050000040000507009920108000034059000507000000",
            "020000000000700003074080000000003002080040010600500000000010780500009000000000040",
            "300000000970010000600583000200000900040020000000600008006040000090000307000001402",
            "000090200004000000100308000052007006000000000400100987000605001000000600008070000",
            "000000000900000084062300050000600200070102030003007000020009140690000007000000000",
            "000000000079050180800000007007306800450708096003402700700000009016030420000000000",
            "000000000001900500960001070000700004000030000300005000080200049002008700000000000"
        }},
        {"Hard", {
            "400000805030000000000700000020000060000080400000010000000603070500200000104000000",
            "520006000000000701300000000000400800600000050000000000041800000000030020008700000",
            "600000803040700000000000000000504070300200000106000000020000050000080600000010000",
            "000000000079050180800000007007306800450708096003402700700000009016030420000000000",
            "000000000001900500960001070000700004000030000300005000080200049002008700000000000",
            "000000052080000000000700340000900000000080000070000600004600800000000000000000000",
            "000075400000000008080190000300001060000000034000068170204000603900000401530000000",
            "300000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        }},
        {"Expert", {
            "800000000003600000070090200050007000000045700000100030001000068008500010090000400",
            "000000085000210009960080100500800016000000000890006007009070052300054000480000000",
            "000200000000060700700000009800000040010000050040000003200000004003010000000008000",
            "000070100000800005900200000000400800100000002005003000000005009700006000002010000",
            "000000801700200000000600000000500070010000050060003000000009000000001006803000000",
            "000000000900000084062300050000600200070102030003007000020009140690000007000000000",
            "000000000079050180800000007007306800450708096003402700700000009016030420000000000",
            "000000000001900500960001070000700004000030000300005000080200049002008700000000000",
            "000000052080000000000700340000900000000080000070000600004600800000000000000000000"
        }}
    };
    
    // Select a random puzzle for the chosen difficulty
    auto it = puzzles.find(difficulty);
    if (it != puzzles.end()) {
        int index = static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(it->second.size())));
        importFromString(it->second[index]);
    } else {
        // Fallback to an easy puzzle if difficulty not found
        importFromString(puzzles.at("Easy")[0]);
    }
    
    // Save initial state
    pushState();
    
    // Reset score and mistakes
    emit scoreReset();
    emit mistakesReset();
} 