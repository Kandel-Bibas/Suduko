#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QShortcut>
#include <QFileDialog>
#include <QTime>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpacerItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , gridWidget(nullptr)
    , solveButton(nullptr)
    , clearButton(nullptr)
    , validateButton(nullptr)
    , loadExampleButton(nullptr)
    , centralWidget(nullptr)
    , mainLayout(nullptr)
    , messageLabel(nullptr)
    , statsLabel(nullptr)
    , timerLabel(nullptr)
    , mistakesLabel(nullptr)
    , scoreLabel(nullptr)
    , solver(std::vector<std::vector<int>>(9, std::vector<int>(9, 0)))
    , settings("SudokuSolver", "Preferences")
    , isDarkTheme(false)
    , puzzlesSolved(0)
    , bestTime(-1)
    , mistakes(0)
    , score(0)
    , gameTimer(nullptr)
{
    qDebug() << "Starting MainWindow initialization";
    
    // Initialize game timer
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, [this]() {
        if (timerLabel) {
            QTime time = QTime::fromString(timerLabel->text(), "mm:ss");
            time = time.addSecs(1);
            timerLabel->setText(time.toString("mm:ss"));
        }
    });
    
    setWindowTitle("Sudoku Solver");
    setMinimumSize(1200, 800);
    
    // Set window background
    setStyleSheet("QMainWindow { background-color: #f8f9fa; }");
    
    qDebug() << "Creating central widget";
    // Create central widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // Create message label
    messageLabel = new QLabel(this);
    messageLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-size: 16px;"
        "   padding: 15px;"
        "   border-radius: 8px;"
        "   background-color: #636e72;"
        "}"
    );
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->hide();
    mainLayout->addWidget(messageLabel);
    
    qDebug() << "Creating top section";
    // Top section with title and difficulty
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // Title
    QLabel* titleLabel = new QLabel("Sudoku Solver", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #2d3436;"
        "   font-size: 32px;"
        "   font-weight: bold;"
        "}"
    );
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // Theme switch button
    QPushButton* themeButton = createIconButton(isDarkTheme ? "🌙" : "☀️", "Toggle theme");
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    
    // Difficulty selector
    QComboBox* difficultyBox = new QComboBox(this);
    difficultyBox->addItems({"Easy", "Medium", "Hard", "Expert", "Master", "Extreme"});
    difficultyBox->setStyleSheet(
        "QComboBox {"
        "   background-color: white;"
        "   border: 1px solid #dfe6e9;"
        "   border-radius: 8px;"
        "   padding: 8px 15px;"
        "   min-width: 120px;"
        "   color: #2d3436;"
        "   font-size: 16px;"
        "}"
    );
    
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(themeButton);
    topLayout->addWidget(new QLabel("Difficulty:", this));
    topLayout->addWidget(difficultyBox);
    
    mainLayout->addLayout(topLayout);
    
    qDebug() << "Creating stats layout";
    // Game stats layout
    QHBoxLayout* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    // Timer
    timerLabel = new QLabel("00:00", this);
    timerLabel->setStyleSheet(
        "QLabel {"
        "   color: #2d3436;"
        "   font-size: 18px;"
        "}"
    );
    
    // Mistakes counter
    mistakesLabel = new QLabel("Mistakes: 0/3", this);
    mistakesLabel->setStyleSheet(
        "QLabel {"
        "   color: #2d3436;"
        "   font-size: 18px;"
        "}"
    );
    
    // Score
    scoreLabel = new QLabel("Score: 0", this);
    scoreLabel->setStyleSheet(
        "QLabel {"
        "   color: #2d3436;"
        "   font-size: 18px;"
        "}"
    );
    
    // Stats label
    statsLabel = new QLabel(this);
    statsLabel->setStyleSheet(
        "QLabel {"
        "   color: #636e72;"
        "   font-size: 14px;"
        "}"
    );
    statsLabel->setAlignment(Qt::AlignCenter);
    
    statsLayout->addWidget(timerLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(mistakesLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(scoreLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(statsLabel);
    
    mainLayout->addLayout(statsLayout);
    
    qDebug() << "Creating grid and controls";
    // Grid and controls layout
    QHBoxLayout* gameLayout = new QHBoxLayout();
    
    // Grid widget
    gridWidget = new SudokuGrid(this);
    gameLayout->addWidget(gridWidget);
    
    // Right side controls
    QVBoxLayout* controlsLayout = new QVBoxLayout();
    controlsLayout->setSpacing(15);
    
    // Game controls
    QHBoxLayout* gameControlsLayout = new QHBoxLayout();
    gameControlsLayout->setSpacing(10);
    
    // Control buttons with icons
    QPushButton* undoButton = createIconButton("↺", "Undo");
    QPushButton* eraseButton = createIconButton("⌫", "Erase");
    QPushButton* notesButton = createIconButton("✎", "Notes");
    QPushButton* hintButton = createIconButton("💡", "Hint");
    
    connect(undoButton, &QPushButton::clicked, gridWidget, &SudokuGrid::undoMove);
    connect(eraseButton, &QPushButton::clicked, [this]() {
        if (gridWidget->getCurrentCell()) {
            gridWidget->getCurrentCell()->clear();
        }
    });
    connect(notesButton, &QPushButton::clicked, [this, notesButton]() {
        gridWidget->setNotesMode(!gridWidget->isNotesMode());
        notesButton->setStyleSheet(
            gridWidget->isNotesMode() ? 
            "background-color: #0984e3; color: white;" :
            "background-color: #f1f2f6; color: #2d3436;"
        );
    });
    connect(hintButton, &QPushButton::clicked, [this]() {
        gridWidget->showHint();
    });
    
    gameControlsLayout->addWidget(undoButton);
    gameControlsLayout->addWidget(eraseButton);
    gameControlsLayout->addWidget(notesButton);
    gameControlsLayout->addWidget(hintButton);
    
    controlsLayout->addLayout(gameControlsLayout);
    
    // Number pad
    QGridLayout* numberPadLayout = new QGridLayout();
    numberPadLayout->setSpacing(10);
    
    for (int i = 0; i < 9; ++i) {
        QPushButton* numButton = createNumberButton(QString::number(i + 1));
        connect(numButton, &QPushButton::clicked, [this, i]() {
            if (gridWidget->getCurrentCell()) {
                if (gridWidget->isNotesMode()) {
                    gridWidget->toggleNote(gridWidget->getCurrentCell(), i + 1);
                } else {
                    gridWidget->getCurrentCell()->setText(QString::number(i + 1));
                }
            }
        });
        numberPadLayout->addWidget(numButton, i / 3, i % 3);
    }
    
    controlsLayout->addLayout(numberPadLayout);
    
    // New Game button
    QPushButton* newGameButton = new QPushButton("New Game", this);
    newGameButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4a69bd;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 15px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3c55a5;"
        "}"
    );
    
    connect(newGameButton, &QPushButton::clicked, [this, difficultyBox]() {
        QString difficulty = difficultyBox->currentText();
        gridWidget->newGame(difficulty);
        gameTimer->start(1000);  // Start timer
        mistakes = 0;
        score = 0;
        updateStats();
    });
    
    controlsLayout->addWidget(newGameButton);
    controlsLayout->addStretch();
    
    gameLayout->addLayout(controlsLayout);
    mainLayout->addLayout(gameLayout);
    
    qDebug() << "Creating action buttons";
    // Bottom action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(15);
    
    loadExampleButton = createStyledButton("Load Example", "#ff9f43", "L");
    validateButton = createStyledButton("Validate", "#00b894", "V");
    solveButton = createStyledButton("Solve", "#0984e3", "S");
    clearButton = createStyledButton("Clear", "#d63031", "C");
    
    actionLayout->addWidget(loadExampleButton);
    actionLayout->addWidget(validateButton);
    actionLayout->addWidget(solveButton);
    actionLayout->addWidget(clearButton);
    
    mainLayout->addLayout(actionLayout);
    
    qDebug() << "Connecting signals";
    // Connect signals
    connect(loadExampleButton, &QPushButton::clicked, this, &MainWindow::onLoadExampleClicked);
    connect(validateButton, &QPushButton::clicked, this, &MainWindow::onValidateClicked);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(gridWidget, &SudokuGrid::gridChanged, this, &MainWindow::onGridChanged);
    connect(gridWidget, &SudokuGrid::validityChanged, this, &MainWindow::onValidityChanged);
    
    // Initialize
    isDarkTheme = settings.value("darkTheme", false).toBool();
    puzzlesSolved = settings.value("puzzlesSolved", 0).toInt();
    bestTime = settings.value("bestTime", -1).toInt();
    
    updateStats();
    applyTheme(isDarkTheme);
    
    qDebug() << "MainWindow initialization complete";
}

QPushButton* MainWindow::createIconButton(const QString& icon, const QString& tooltip) {
    QPushButton* button = new QPushButton(icon, this);
    button->setFixedSize(45, 45);
    button->setToolTip(tooltip);
    button->setFont(QFont("SF Pro Display", 18));
    button->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 12px;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: %4;
        }
        QPushButton:checked {
            background-color: #0984e3;
            color: white;
        }
    )").arg(isDarkTheme ? "#2d3436" : "#ffffff")
       .arg(isDarkTheme ? "#ffffff" : "#2d3436")
       .arg(isDarkTheme ? "#485460" : "#f5f6fa")
       .arg(isDarkTheme ? "#1e272e" : "#dfe6e9"));
    
    return button;
}

QPushButton* MainWindow::createNumberButton(const QString& number) {
    QPushButton* button = new QPushButton(number, this);
    button->setFixedSize(55, 55);
    button->setFont(QFont("SF Pro Display", 24, QFont::DemiBold));
    button->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 12px;
            font-size: 24px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: %4;
        }
    )").arg(isDarkTheme ? "#2d3436" : "#ffffff")
       .arg(isDarkTheme ? "#ffffff" : "#2d3436")
       .arg(isDarkTheme ? "#485460" : "#f5f6fa")
       .arg(isDarkTheme ? "#1e272e" : "#dfe6e9"));
    
    connect(button, &QPushButton::clicked, this, [this, number]() {
        if (gridWidget && gridWidget->getCurrentCell()) {
            if (gridWidget->isNotesMode()) {
                gridWidget->toggleNote(gridWidget->getCurrentCell(), number.toInt());
            } else {
                gridWidget->getCurrentCell()->setText(number);
            }
        }
    });
    
    return button;
}

QPushButton* MainWindow::createStyledButton(const QString& text, const QString& color, const QString& shortcut) {
    QPushButton* button = new QPushButton(text, this);
    
    QString baseStyle = QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: white;"
        "   border: none;"
        "   padding: 15px 30px;"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: %3;"
        "}"
    );
    
    button->setStyleSheet(baseStyle.arg(color).arg(adjustColor(color, 1.1)).arg(adjustColor(color, 0.9)));
    button->setShortcut(QKeySequence(QString("Ctrl+%1").arg(shortcut)));
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(50);
    
    return button;
}

QString MainWindow::adjustColor(const QString& color, double factor) {
    QColor c(color);
    return QString("rgb(%1,%2,%3)").arg(
        qBound(0, int(c.red() * factor), 255))
        .arg(qBound(0, int(c.green() * factor), 255))
        .arg(qBound(0, int(c.blue() * factor), 255));
}

void MainWindow::showMessage(const QString& message, const QString& color) {
    qDebug() << "Showing message:" << message;
    if (messageLabel) {
        messageLabel->setStyleSheet(QString("QLabel { background-color: %1; color: white; font-size: 16px; padding: 15px; border-radius: 8px; }").arg(color));
        messageLabel->setText(message);
        messageLabel->show();
        
        // Auto-hide after 3 seconds
        QTimer::singleShot(3000, messageLabel, &QLabel::hide);
    }
}

void MainWindow::applyTheme(bool isDark) {
    isDarkTheme = isDark;
    QString bgColor = isDark ? "#1e272e" : "#ffffff";
    QString textColor = isDark ? "#ffffff" : "#2d3436";
    QString gridBgColor = isDark ? "#2d3436" : "#f5f6fa";
    QString borderColor = isDark ? "#485460" : "#dfe6e9";
    
    // Main window style
    setStyleSheet(QString(R"(
        QMainWindow {
            background-color: %1;
            color: %2;
        }
        QLabel {
            color: %2;
            font-size: 16px;
        }
        QComboBox {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 4px;
            padding: 5px;
            min-width: 100px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: url(:/icons/%5);
            width: 12px;
            height: 12px;
        }
    )").arg(bgColor, textColor, gridBgColor, borderColor, isDark ? "down-arrow-light" : "down-arrow-dark"));

    // Apply theme to grid
    if (gridWidget) {
        gridWidget->applyTheme(isDark);
    }

    // Update button styles
    if (loadExampleButton) loadExampleButton->setStyleSheet(getButtonStyle("#ff9f43"));
    if (validateButton) validateButton->setStyleSheet(getButtonStyle("#00b894"));
    if (solveButton) solveButton->setStyleSheet(getButtonStyle("#0984e3"));
    if (clearButton) clearButton->setStyleSheet(getButtonStyle("#d63031"));

    // Update stats labels
    QString statsStyle = QString("QLabel { color: %1; font-size: 14px; }").arg(textColor);
    if (timerLabel) timerLabel->setStyleSheet(statsStyle);
    if (mistakesLabel) mistakesLabel->setStyleSheet(statsStyle);
    if (scoreLabel) scoreLabel->setStyleSheet(statsStyle);
    if (statsLabel) statsLabel->setStyleSheet(statsStyle);
}

QString MainWindow::getButtonStyle(const QString& color) {
    QString hoverColor = adjustColor(color, isDarkTheme ? 1.1 : 0.9);
    QString pressedColor = adjustColor(color, isDarkTheme ? 0.9 : 1.1);
    QString textColor = isDarkTheme ? "#ffffff" : "#2d3436";
    
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px;
            font-size: 16px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
        QPushButton:disabled {
            background-color: #636e72;
        }
    )").arg(color, hoverColor, pressedColor);
}

void MainWindow::onSolveClicked() {
    qDebug() << "Solving puzzle";
    if (!gridWidget || !gridWidget->isValid()) {
        showMessage("Cannot solve invalid puzzle", "#d63031");
        return;
    }
    
    solver.setGrid(gridWidget->getGrid());
    solveStartTime = QTime::currentTime();
    
    if (solver.solve()) {
        gridWidget->setGrid(solver.getGrid());
        int timeInSeconds = solveStartTime.secsTo(QTime::currentTime());
        puzzlesSolved++;
        if (bestTime == -1 || timeInSeconds < bestTime) {
            bestTime = timeInSeconds;
        }
        settings.setValue("puzzlesSolved", puzzlesSolved);
        settings.setValue("bestTime", bestTime);
        updateStats();
        showMessage(QString("🎉 Puzzle solved in %1!").arg(QTime(0, 0).addSecs(timeInSeconds).toString("mm:ss")), "#00b894");
    } else {
        showMessage("No solution exists for this puzzle", "#d63031");
    }
}

void MainWindow::onClearClicked() {
    qDebug() << "Clearing grid";
    if (gridWidget) {
        gridWidget->clear();
        showMessage("Grid cleared", "#636e72");
    }
}

void MainWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    settings.setValue("darkTheme", isDarkTheme);
    applyTheme(isDarkTheme);
}

void MainWindow::saveGrid() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Puzzle", "", "Sudoku Files (*.sudoku)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << gridWidget->exportToString();
            showMessage("💾 Puzzle saved successfully!", "#3498db");
        }
    }
}

void MainWindow::loadGrid() {
    QString fileName = QFileDialog::getOpenFileName(this, "Load Puzzle", "", "Sudoku Files (*.sudoku)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString data = in.readAll();
            if (gridWidget->importFromString(data)) {
                showMessage("📂 Puzzle loaded successfully!", "#3498db");
            } else {
                showMessage("❌ Invalid puzzle file!", "#e74c3c");
            }
        }
    }
}

void MainWindow::toggleFullscreen() {
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::updateStats() {
    if (statsLabel) {
        QString statsText = QString("Puzzles Solved: %1 | Best Time: %2")
            .arg(puzzlesSolved)
            .arg(bestTime == -1 ? "--:--" : QTime(0, 0).addSecs(bestTime).toString("mm:ss"));
        statsLabel->setText(statsText);
    }
}

void MainWindow::showStats() {
    QMessageBox stats(this);
    stats.setWindowTitle("Statistics");
    stats.setText("<h2>Your Sudoku Stats</h2>");
    stats.setInformativeText(QString(
        "Puzzles Solved: %1<br>"
        "Best Time: %2<br>"
        "<br>"
        "<b>Current Session:</b><br>"
        "Moves Made: %3<br>"
        "Hints Used: %4"
    ).arg(puzzlesSolved)
     .arg(bestTime > 0 ? QTime(0, 0).addSecs(bestTime).toString("mm:ss") : "--:--")
     .arg(gridWidget->getCurrentState().moveCount)
     .arg(settings.value("hintsUsed", 0).toInt()));

    stats.setStyleSheet(isDarkTheme ? 
        "QMessageBox { background-color: #1a1a2e; } QLabel { color: white; }" :
        "QMessageBox { background-color: white; } QLabel { color: #2c3e50; }");
    stats.exec();
}

void MainWindow::onLoadExampleClicked() {
    qDebug() << "Loading example puzzle";
    if (gridWidget) {
        gridWidget->loadExample();
        showMessage("📋 Example puzzle loaded", "#ff9f43");
    }
}

void MainWindow::onValidateClicked() {
    qDebug() << "Validating puzzle";
    if (gridWidget) {
        gridWidget->highlightConflicts();
        if (gridWidget->isValid()) {
            showMessage("✓ Puzzle is valid!", "#00b894");
        } else {
            showMessage("⚠️ Found conflicts in the puzzle", "#d63031");
        }
    }
}

void MainWindow::onGridChanged() {
    qDebug() << "Grid changed";
    if (gridWidget) {
        solver.setGrid(gridWidget->getGrid());
    }
}

void MainWindow::onValidityChanged(bool isValid) {
    qDebug() << "Validity changed:" << isValid;
    if (solveButton) {
        solveButton->setEnabled(isValid);
        if (!isValid) {
            showMessage("⚠️ Invalid puzzle configuration", "#d63031");
        }
    }
} 