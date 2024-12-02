#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QTimer>
#include <QRandomGenerator>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include "solver.h"
#include <vector>
#include <optional>
#include <map>

class SudokuGrid : public QWidget {
    Q_OBJECT

public:
    struct GridState {
        std::vector<std::vector<int>> values;
        std::map<QLineEdit*, std::vector<int>> notes;
        int moveCount;
    };

    explicit SudokuGrid(QWidget *parent = nullptr);
    std::vector<std::vector<int>> getGrid() const;
    void setGrid(const std::vector<std::vector<int>> &grid);
    void clear();
    bool isValid() const;
    void highlightConflicts();
    void loadExample();
    bool isFull() const;
    void applyTheme(bool isDark);
    void setHints(bool enabled);
    void undoMove();
    void redoMove();
    void saveState();
    QString exportToString() const;
    bool importFromString(const QString& data);
    GridState getCurrentState() const;
    void newGame(const QString& difficulty);
    void showHint();
    QLineEdit* getCurrentCell() const { return currentCell; }
    bool isNotesMode() const;
    void setNotesMode(bool enabled);
    void toggleNote(QLineEdit* cell, int number);
    
    // New methods for file operations
    bool savePuzzleToFile(const QString& filename = QString());
    bool loadPuzzleFromFile(const QString& filename);
    QStringList getSavedPuzzleFiles() const;

signals:
    void gridChanged();
    void validityChanged(bool isValid);
    void moveAdded();
    void puzzleSolved(int timeInSeconds);
    void cellSelected(QLineEdit* cell);
    void puzzleSaved(const QString& filename);
    void puzzleLoaded(const QString& filename);
    void mistakeAdded();  // Signal when a mistake is made
    void mistakesReset(); // Signal to reset mistakes counter
    void scoreIncreased(int points);  // Signal when score should increase
    void scoreReset();    // Signal to reset score

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QGridLayout *layout;
    std::vector<std::vector<QLineEdit*>> cells;
    std::vector<GridState> undoStack;
    std::vector<GridState> redoStack;
    QTimer *hintTimer;
    bool hintsEnabled;
    bool isDarkTheme;
    bool notesMode;
    QLineEdit* currentCell;
    std::map<QLineEdit*, std::vector<int>> notes;
    SudokuSolver solver;
    QString savesDirectory;

    void createGrid();
    void styleCell(QLineEdit* cell, int row, int col);
    void validateInput(QLineEdit* cell);
    bool checkRowValid(int row) const;
    bool checkColumnValid(int col) const;
    bool checkBoxValid(int startRow, int startCol) const;
    void clearHighlighting();
    std::optional<int> getCellValue(int row, int col) const;
    void animateCell(QLineEdit* cell, const QString& color);
    void showPossibleValues(int row, int col);
    void checkCompletion();
    void pushState();
    void applyState(const GridState& state);
    void updateCellNotes(QLineEdit* cell);
    void highlightCell(QLineEdit* cell, bool isError);
    void ensureSavesDirectoryExists();
    void selectCell(QLineEdit* cell);
}; 