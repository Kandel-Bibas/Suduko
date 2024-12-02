#include "mainwindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , solver(std::vector<std::vector<int>>(9, std::vector<int>(9, 0)))
{
    setWindowTitle("Sudoku Solver");
    setMinimumSize(600, 800);
    setStyleSheet("QMainWindow { background-color: #2C3E50; }");

    // Create menu bar
    createMenus();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Add title
    QLabel* titleLabel = new QLabel("Sudoku Solver", this);
    titleLabel->setStyleSheet(
        "QLabel { "
        "   color: white; "
        "   font-size: 32px; "
        "   font-weight: bold; "
        "   margin: 20px; "
        "   font-family: 'Arial', sans-serif; "
        "}"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Create message label
    messageLabel = new QLabel(this);
    messageLabel->setStyleSheet(
        "QLabel { "
        "   color: white; "
        "   font-size: 16px; "
        "   padding: 10px; "
        "   border-radius: 5px; "
        "   background-color: #34495E; "
        "   margin: 10px; "
        "}"
    );
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->hide();
    mainLayout->addWidget(messageLabel);
    
    // Create a frame for the grid
    QFrame* gridFrame = new QFrame(this);
    gridFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #34495E; "
        "   border-radius: 10px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout* gridFrameLayout = new QVBoxLayout(gridFrame);
    
    // Create grid widget
    gridWidget = new SudokuGrid(this);
    gridFrameLayout->addWidget(gridWidget);
    mainLayout->addWidget(gridFrame);
    
    // Create button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    
    // Create buttons
    solveButton = new QPushButton("Solve", this);
    clearButton = new QPushButton("Clear", this);
    QPushButton* validateButton = new QPushButton("Validate", this);
    QPushButton* loadExampleButton = new QPushButton("Load Example", this);
    
    // Style buttons
    QString buttonStyle = 
        "QPushButton { "
        "   padding: 15px 30px; "
        "   border-radius: 8px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   font-family: 'Arial', sans-serif; "
        "   min-width: 120px; "
        "} "
        "QPushButton:hover { "
        "   opacity: 0.9; "
        "} "
        "QPushButton:pressed { "
        "   opacity: 0.7; "
        "}";
    
    loadExampleButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #F39C12; color: white; }");
    validateButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #3498DB; color: white; }");
    solveButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #2ECC71; color: white; }");
    clearButton->setStyleSheet(buttonStyle + "QPushButton { background-color: #E74C3C; color: white; }");
    
    buttonLayout->addWidget(loadExampleButton);
    buttonLayout->addWidget(validateButton);
    buttonLayout->addWidget(solveButton);
    buttonLayout->addWidget(clearButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(validateButton, &QPushButton::clicked, this, [this]() {
        gridWidget->highlightConflicts();
        if (gridWidget->isValid()) {
            showMessage("Puzzle is valid!", "#2ECC71");
        } else {
            showMessage("Puzzle contains conflicts!", "#E74C3C");
        }
    });
    connect(loadExampleButton, &QPushButton::clicked, this, [this]() {
        gridWidget->loadExample();
        showMessage("Example puzzle loaded", "#F39C12");
    });
    
    connect(gridWidget, &SudokuGrid::gridChanged, this, [this]() {
        messageLabel->hide();
    });
    
    connect(gridWidget, &SudokuGrid::validityChanged, this, [this](bool isValid) {
        solveButton->setEnabled(isValid);
        if (!isValid) {
            showMessage("Invalid puzzle configuration", "#E74C3C");
        }
    });
}

void MainWindow::showMessage(const QString& message, const QString& color) {
    messageLabel->setStyleSheet(
        QString("QLabel { "
                "   color: white; "
                "   font-size: 16px; "
                "   padding: 10px; "
                "   border-radius: 5px; "
                "   background-color: %1; "
                "   margin: 10px; "
                "}").arg(color)
    );
    messageLabel->setText(message);
    messageLabel->show();
}

void MainWindow::createMenus() {
    menuBar()->setStyleSheet(
        "QMenuBar { "
        "   background-color: #34495E; "
        "   color: white; "
        "} "
        "QMenuBar::item:selected { "
        "   background-color: #2C3E50; "
        "} "
        "QMenu { "
        "   background-color: #34495E; "
        "   color: white; "
        "   border: 1px solid #2C3E50; "
        "} "
        "QMenu::item:selected { "
        "   background-color: #2C3E50; "
        "}"
    );
    
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
        showMessage("The current puzzle configuration is invalid. Please fix the conflicts first.", "#E74C3C");
        return;
    }
    
    auto grid = gridWidget->getGrid();
    solver.setGrid(grid);
    
    if (solver.solve()) {
        gridWidget->setGrid(solver.getGrid());
        showMessage("Puzzle solved successfully!", "#2ECC71");
    } else {
        showMessage("No solution exists for this puzzle!", "#E74C3C");
    }
}

void MainWindow::onClearClicked() {
    gridWidget->clear();
    showMessage("Grid cleared", "#3498DB");
} 