#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <vector>
#include <optional>

class SudokuGrid : public QWidget {
    Q_OBJECT

public:
    explicit SudokuGrid(QWidget *parent = nullptr);
    std::vector<std::vector<int>> getGrid() const;
    void setGrid(const std::vector<std::vector<int>> &grid);
    void clear();
    bool isValid() const;
    void highlightConflicts();
    void loadExample();
    bool isFull() const;

signals:
    void gridChanged();
    void validityChanged(bool isValid);

private:
    QGridLayout *layout;
    std::vector<std::vector<QLineEdit*>> cells;
    void createGrid();
    void styleCell(QLineEdit* cell, int row, int col);
    void validateInput(QLineEdit* cell);
    bool checkRowValid(int row) const;
    bool checkColumnValid(int col) const;
    bool checkBoxValid(int startRow, int startCol) const;
    void clearHighlighting();
    std::optional<int> getCellValue(int row, int col) const;
}; 