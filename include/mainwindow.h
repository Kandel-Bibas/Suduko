#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include "sudokugrid.h"
#include "solver.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onSolveClicked();
    void onClearClicked();

private:
    SudokuGrid *gridWidget;
    QPushButton *solveButton;
    QPushButton *clearButton;
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    SudokuSolver solver;
}; 