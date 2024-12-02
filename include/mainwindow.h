#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
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
    void createMenus();
    void showMessage(const QString& message, const QString& color);
    
    SudokuGrid *gridWidget;
    QPushButton *solveButton;
    QPushButton *clearButton;
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QLabel *messageLabel;
    SudokuSolver solver;
}; 