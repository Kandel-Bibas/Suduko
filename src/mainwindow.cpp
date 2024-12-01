#include "mainwindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , solver(std::vector<std::vector<int>>(9, std::vector<int>(9, 0)))
{
    setWindowTitle("Sudoku Solver");
    setMinimumSize(500, 600);

    // Create menu bar
    createMenus();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    
    // Add title
    QLabel* titleLabel = new QLabel("Sudoku Solver", this);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; margin: 10px; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Create grid widget
    gridWidget = new SudokuGrid(this);
    mainLayout->addWidget(gridWidget);
    
    // Create button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // Create buttons
    solveButton = new QPushButton("Solve", this);
    clearButton = new QPushButton("Clear", this);
    QPushButton* validateButton = new QPushButton("Validate", this);
    QPushButton* loadExampleButton = new QPushButton("Load Example", this);
    
    // Style buttons
    QString buttonStyle = "QPushButton { padding: 10px; border-radius: 5px; min-width: 100px; }";
    solveButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #4CAF50; color: white; } QPushButton:hover { background-color: #45a049; }");
    clearButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #f44336; color: white; } QPushButton:hover { background-color: #da190b; }");
    validateButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #2196F3; color: white; } QPushButton:hover { background-color: #1976D2; }");
    loadExampleButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #FF9800; color: white; } QPushButton:hover { background-color: #F57C00; }");
    
    buttonLayout->addWidget(loadExampleButton);
    buttonLayout->addWidget(validateButton);
    buttonLayout->addWidget(solveButton);
    buttonLayout->addWidget(clearButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Create status bar
    statusBar()->showMessage("Ready");
    
    // Connect signals
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(validateButton, &QPushButton::clicked, this, [this]() {
        gridWidget->highlightConflicts();
        if (gridWidget->isValid()) {
            statusBar()->showMessage("Puzzle is valid!", 3000);
        } else {
            statusBar()->showMessage("Puzzle contains conflicts!", 3000);
        }
    });
    connect(loadExampleButton, &QPushButton::clicked, this, [this]() {
        gridWidget->loadExample();
        statusBar()->showMessage("Example puzzle loaded", 3000);
    });
    
    connect(gridWidget, &SudokuGrid::gridChanged, this, [this]() {
        statusBar()->showMessage("Grid updated", 1000);
    });
    
    connect(gridWidget, &SudokuGrid::validityChanged, this, [this](bool isValid) {
        solveButton->setEnabled(isValid);
        if (!isValid) {
            statusBar()->showMessage("Invalid puzzle configuration", 3000);
        }
    });
}

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *newAction = fileMenu->addAction("&New");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onClearClicked);
    
    QAction *loadExampleAction = fileMenu->addAction("Load &Example");
    connect(loadExampleAction, &QAction::triggered, this, [this]() {
        gridWidget->loadExample();
    });
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Sudoku Solver",
            "Sudoku Solver v1.0\n\n"
            "A modern Sudoku puzzle solver using the Ternary Search Tree algorithm.\n\n"
            "Features:\n"
            "- Real-time validation\n"
            "- Conflict highlighting\n"
            "- Fast solving algorithm\n"
            "- Modern user interface\n\n"
            "© 2024 All rights reserved.");
    });
}

void MainWindow::onSolveClicked() {
    if (!gridWidget->isValid()) {
        QMessageBox::warning(this, "Invalid Puzzle", "The current puzzle configuration is invalid. Please fix the conflicts first.");
        return;
    }
    
    auto grid = gridWidget->getGrid();
    solver.setGrid(grid);
    
    if (solver.solve()) {
        gridWidget->setGrid(solver.getGrid());
        statusBar()->showMessage("Puzzle solved successfully!", 3000);
    } else {
        QMessageBox::warning(this, "No Solution", "No solution exists for this puzzle!");
        statusBar()->showMessage("No solution found", 3000);
    }
}

void MainWindow::onClearClicked() {
    gridWidget->clear();
    statusBar()->showMessage("Grid cleared", 3000);
} 