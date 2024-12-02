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
    , isDarkTheme(settings.value("darkTheme", false).toBool())
    , puzzlesSolved(settings.value("puzzlesSolved", 0).toInt())
    , bestTime(settings.value("bestTime", -1).toInt())
    , solveStartTime(QTime::currentTime())
    , mistakes(0)
    , score(0)
    , gameTimer(new QTimer(this))
{
    // Set window properties
    setWindowTitle("Sudoku Solver");
    setMinimumSize(1000, 800);
    
    // Initialize central widget and main layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(32, 24, 32, 24);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(20);
    
    // Left side: Title and difficulty
    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(16);
    
    QLabel* titleLabel = new QLabel("Sudoku Solver", this);
    titleLabel->setFont(QFont("SF Pro Display", 24, QFont::Bold));
    
    QComboBox* difficultyBox = new QComboBox(this);
    difficultyBox->addItems({"Easy", "Medium", "Hard", "Expert"});
    difficultyBox->setFixedWidth(140);
    difficultyBox->setFont(QFont("SF Pro Display", 14));
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(difficultyBox);
    titleLayout->addStretch();
    
    // Right side: Controls
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(12);
    
    QPushButton* themeButton = createIconButton(isDarkTheme ? "☀️" : "🌙", "Toggle theme");
    QPushButton* saveButton = createIconButton("💾", "Save puzzle");
    QPushButton* loadButton = createIconButton("📂", "Load puzzle");
    
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveGrid);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadGrid);
    
    controlsLayout->addWidget(themeButton);
    controlsLayout->addWidget(saveButton);
    controlsLayout->addWidget(loadButton);
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addLayout(controlsLayout);
    
    mainLayout->addLayout(headerLayout);
    
    // Stats bar
    QFrame* statsFrame = new QFrame(this);
    statsFrame->setFrameShape(QFrame::NoFrame);
    statsFrame->setStyleSheet("QFrame { background-color: " + QString(isDarkTheme ? "#2d3436" : "#f5f6fa") + "; border-radius: 12px; }");
    
    QHBoxLayout* statsLayout = new QHBoxLayout(statsFrame);
    statsLayout->setSpacing(40);
    statsLayout->setContentsMargins(24, 16, 24, 16);
    
    timerLabel = new QLabel("00:00", this);
    mistakesLabel = new QLabel("Mistakes: 0/3", this);
    scoreLabel = new QLabel("Score: 0", this);
    statsLabel = new QLabel(this);
    
    QFont statsFont("SF Pro Display", 14);
    timerLabel->setFont(statsFont);
    mistakesLabel->setFont(statsFont);
    scoreLabel->setFont(statsFont);
    statsLabel->setFont(statsFont);
    
    updateStats();
    
    statsLayout->addWidget(timerLabel);
    statsLayout->addWidget(mistakesLabel);
    statsLayout->addWidget(scoreLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(statsLabel);
    
    mainLayout->addWidget(statsFrame);
    
    // Game area container
    QWidget* gameContainer = new QWidget(this);
    QHBoxLayout* gameLayout = new QHBoxLayout(gameContainer);
    gameLayout->setSpacing(40);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create grid
    gridWidget = new SudokuGrid(this);
    gridWidget->setMinimumSize(500, 500);
    connect(gridWidget, &SudokuGrid::gridChanged, this, &MainWindow::onGridChanged);
    connect(gridWidget, &SudokuGrid::validityChanged, this, &MainWindow::onValidityChanged);
    connect(gridWidget, &SudokuGrid::puzzleSolved, [this](int timeInSeconds) {
        gameTimer->stop();
        showMessage("🎉 Puzzle solved!", "#00b894");
        puzzlesSolved++;
        if (bestTime == -1 || timeInSeconds < bestTime) {
            bestTime = timeInSeconds;
        }
        settings.setValue("puzzlesSolved", puzzlesSolved);
        settings.setValue("bestTime", bestTime);
        updateStats();
    });
    
    // Connect score and mistake signals
    connect(gridWidget, &SudokuGrid::mistakeAdded, [this]() {
        mistakes++;
        mistakesLabel->setText(QString("Mistakes: %1/3").arg(mistakes));
        if (mistakes >= 3) {
            showMessage("❌ Game Over - Too many mistakes!", "#ff6b6b");
            gridWidget->setEnabled(false);
        }
    });
    
    connect(gridWidget, &SudokuGrid::mistakesReset, [this]() {
        mistakes = 0;
        mistakesLabel->setText("Mistakes: 0/3");
        gridWidget->setEnabled(true);
    });
    
    connect(gridWidget, &SudokuGrid::scoreIncreased, [this](int points) {
        score += points;
        scoreLabel->setText(QString("Score: %1").arg(score));
    });
    
    connect(gridWidget, &SudokuGrid::scoreReset, [this]() {
        score = 0;
        scoreLabel->setText("Score: 0");
    });
    
    // Right side controls
    QVBoxLayout* rightControls = new QVBoxLayout();
    rightControls->setSpacing(24);
    
    // Game controls
    QHBoxLayout* gameControls = new QHBoxLayout();
    gameControls->setSpacing(12);
    
    QPushButton* undoButton = createIconButton("↩️", "Undo");
    QPushButton* eraseButton = createIconButton("🗑️", "Erase");
    QPushButton* notesButton = createIconButton("✏️", "Notes mode");
    QPushButton* hintButton = createIconButton("💡", "Get hint");
    
    notesButton->setCheckable(true);
    
    connect(undoButton, &QPushButton::clicked, gridWidget, &SudokuGrid::undoMove);
    connect(eraseButton, &QPushButton::clicked, [this]() {
        if (gridWidget && gridWidget->getCurrentCell()) {
            gridWidget->getCurrentCell()->clear();
        }
    });
    connect(notesButton, &QPushButton::toggled, [this](bool checked) {
        if (gridWidget) {
            gridWidget->setNotesMode(checked);
        }
    });
    connect(hintButton, &QPushButton::clicked, gridWidget, &SudokuGrid::showHint);
    
    gameControls->addWidget(undoButton);
    gameControls->addWidget(eraseButton);
    gameControls->addWidget(notesButton);
    gameControls->addWidget(hintButton);
    
    // Number pad
    QGridLayout* numberPad = new QGridLayout();
    numberPad->setSpacing(8);
    for (int i = 0; i < 9; ++i) {
        QPushButton* numButton = createNumberButton(QString::number(i + 1));
        numberPad->addWidget(numButton, i / 3, i % 3);
    }
    
    // New game button
    QPushButton* newGameButton = new QPushButton("New Game", this);
    newGameButton->setFixedHeight(50);
    newGameButton->setFont(QFont("SF Pro Display", 16, QFont::DemiBold));
    newGameButton->setStyleSheet(getButtonStyle("#0984e3"));
    
    connect(newGameButton, &QPushButton::clicked, [this, difficultyBox]() {
        if (gridWidget) {
            gridWidget->newGame(difficultyBox->currentText());
            gameTimer->start(1000);
            timerLabel->setText("00:00");
            mistakes = 0;
            score = 0;
            mistakesLabel->setText("Mistakes: 0/3");
            scoreLabel->setText("Score: 0");
        }
    });
    
    rightControls->addLayout(gameControls);
    rightControls->addLayout(numberPad);
    rightControls->addWidget(newGameButton);
    rightControls->addStretch();
    
    gameLayout->addWidget(gridWidget);
    gameLayout->addLayout(rightControls);
    
    mainLayout->addWidget(gameContainer, 1); // Give it stretch factor of 1
    
    // Message container (placed above action buttons)
    QWidget* messageContainer = new QWidget(this);
    QVBoxLayout* messageLayout = new QVBoxLayout(messageContainer);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    messageLabel = new QLabel(this);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setFont(QFont("SF Pro Display", 14));
    messageLabel->hide();
    messageLayout->addWidget(messageLabel);
    messageContainer->setFixedHeight(50);
    
    mainLayout->addWidget(messageContainer);
    
    // Action buttons at the bottom
    QHBoxLayout* actionButtons = new QHBoxLayout();
    actionButtons->setSpacing(16);
    
    loadExampleButton = createStyledButton("Load Example", "#ff9f43", "L");
    validateButton = createStyledButton("Validate", "#00b894", "V");
    solveButton = createStyledButton("Solve", "#0984e3", "S");
    clearButton = createStyledButton("Clear", "#d63031", "C");
    
    connect(loadExampleButton, &QPushButton::clicked, this, &MainWindow::onLoadExampleClicked);
    connect(validateButton, &QPushButton::clicked, this, &MainWindow::onValidateClicked);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    
    actionButtons->addWidget(loadExampleButton);
    actionButtons->addWidget(validateButton);
    actionButtons->addWidget(solveButton);
    actionButtons->addWidget(clearButton);
    
    mainLayout->addLayout(actionButtons);
    
    // Initialize game timer
    connect(gameTimer, &QTimer::timeout, [this]() {
        if (timerLabel) {
            QTime time = QTime::fromString(timerLabel->text(), "mm:ss");
            time = time.addSecs(1);
            timerLabel->setText(time.toString("mm:ss"));
        }
    });
    
    // Apply initial theme
    applyTheme(isDarkTheme);
}

QPushButton* MainWindow::createIconButton(const QString& icon, const QString& tooltip) {
    QPushButton* button = new QPushButton(icon, this);
    button->setFixedSize(40, 40);
    button->setToolTip(tooltip);
    button->setFont(QFont("SF Pro Display", 16));
    button->setCursor(Qt::PointingHandCursor);
    
    updateButtonStyle(button);
    
    return button;
}

QPushButton* MainWindow::createNumberButton(const QString& number) {
    QPushButton* button = new QPushButton(number, this);
    button->setFixedSize(60, 60);
    button->setFont(QFont("SF Pro Display", 20, QFont::DemiBold));
    button->setCursor(Qt::PointingHandCursor);
    
    updateButtonStyle(button);
    
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

void MainWindow::updateButtonStyle(QPushButton* button) {
    QString bgColor = isDarkTheme ? "#2d3436" : "#ffffff";
    QString textColor = isDarkTheme ? "#ffffff" : "#2d3436";
    QString borderColor = isDarkTheme ? "#485460" : "#dfe6e9";
    QString hoverColor = isDarkTheme ? "#485460" : "#f5f6fa";
    
    button->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: 1.5px solid %3;
            border-radius: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: %4;
            border-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
    )").arg(bgColor, textColor, borderColor, hoverColor));
}

QPushButton* MainWindow::createStyledButton(const QString& text, const QString& color, const QString& shortcut) {
    QPushButton* button = new QPushButton(text, this);
    button->setCursor(Qt::PointingHandCursor);
    button->setFont(QFont("SF Pro Display", 14, QFont::DemiBold));
    button->setFixedHeight(45);
    
    QString hoverColor = adjustColor(color, isDarkTheme ? 1.2 : 0.9);
    QString pressedColor = adjustColor(color, isDarkTheme ? 0.8 : 1.1);
    
    button->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 10px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
        QPushButton:disabled {
            background-color: %4;
            color: %5;
        }
    )").arg(color, hoverColor, pressedColor,
            isDarkTheme ? "#485460" : "#e0e0e0",
            isDarkTheme ? "#8395a7" : "#a0a0a0"));
    
    button->setShortcut(QKeySequence(QString("Ctrl+%1").arg(shortcut)));
    
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
    if (messageLabel) {
        QString bgColor = isDarkTheme ? adjustColor(color, 0.8) : adjustColor(color, 1.1);
        messageLabel->setStyleSheet(QString(R"(
            QLabel {
                background-color: %1;
                color: white;
                font-size: 14px;
                font-weight: 500;
                padding: 12px 24px;
                border-radius: 10px;
            }
        )").arg(bgColor));
        messageLabel->setText(message);
        messageLabel->show();
        
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
        }
        QComboBox {
            background-color: %3;
            color: %2;
            border: 1.5px solid %4;
            border-radius: 10px;
            padding: 8px 16px;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox::down-arrow {
            image: url(:/icons/%5);
            width: 12px;
            height: 12px;
        }
        QComboBox QAbstractItemView {
            background-color: %3;
            color: %2;
            border: 1.5px solid %4;
            border-radius: 10px;
            selection-background-color: %4;
        }
    )").arg(bgColor, textColor, gridBgColor, borderColor,
            isDark ? "down-arrow-light" : "down-arrow-dark"));
    
    // Update all buttons
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        updateButtonStyle(button);
    }
    
    // Apply theme to grid
    if (gridWidget) {
        gridWidget->applyTheme(isDark);
    }
    
    // Update stats frame
    if (statsLabel) {
        QString statsFrameStyle = QString(R"(
            QFrame {
                background-color: %1;
                border-radius: 12px;
            }
            QLabel {
                color: %2;
                font-size: 14px;
                font-weight: 500;
            }
        )").arg(isDark ? "#2d3436" : "#f5f6fa", textColor);
        statsLabel->parentWidget()->setStyleSheet(statsFrameStyle);
    }
    
    // Save theme preference
    settings.setValue("darkTheme", isDark);
}

QString MainWindow::getButtonStyle(const QString& color) {
    QString hoverColor = adjustColor(color, isDarkTheme ? 1.1 : 0.9);
    QString pressedColor = adjustColor(color, isDarkTheme ? 0.9 : 1.1);
    
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
        QPushButton:disabled {
            background-color: #a0a0a0;
            color: #e0e0e0;
        }
    )").arg(color, hoverColor, pressedColor);
}

void MainWindow::onSolveClicked() {
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
    if (!gridWidget) return;
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Puzzle",
        QDir::homePath(),
        "Sudoku Files (*.sudoku)"
    );
    
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".sudoku")) {
            fileName += ".sudoku";
        }
        
        if (gridWidget->savePuzzleToFile(fileName)) {
            showMessage("💾 Puzzle saved successfully!", "#00b894");
        } else {
            showMessage("Failed to save puzzle", "#d63031");
        }
    }
}

void MainWindow::loadGrid() {
    if (!gridWidget) return;
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Puzzle",
        QDir::homePath(),
        "Sudoku Files (*.sudoku)"
    );
    
    if (!fileName.isEmpty()) {
        if (gridWidget->loadPuzzleFromFile(fileName)) {
            showMessage("📂 Puzzle loaded successfully!", "#00b894");
            gameTimer->start(1000);
            mistakes = 0;
            score = 0;
            mistakesLabel->setText("Mistakes: 0/3");
            scoreLabel->setText("Score: 0");
        } else {
            showMessage("Failed to load puzzle", "#d63031");
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
    if (gridWidget) {
        gridWidget->loadExample();
        showMessage("📋 Example puzzle loaded", "#ff9f43");
    }
}

void MainWindow::onValidateClicked() {
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
    if (gridWidget) {
        solver.setGrid(gridWidget->getGrid());
    }
}

void MainWindow::onValidityChanged(bool isValid) {
    if (solveButton) {
        solveButton->setEnabled(isValid);
        if (!isValid) {
            showMessage("⚠️ Invalid puzzle configuration", "#d63031");
        }
    }
}

void MainWindow::createMenus() {
    // Top section with title and controls
    QHBoxLayout* topSection = new QHBoxLayout();
    topSection->setSpacing(15);
    
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
    
    // Right side controls
    QHBoxLayout* rightControls = new QHBoxLayout();
    rightControls->setSpacing(10);
    
    // Theme toggle button
    QPushButton* themeButton = createIconButton(isDarkTheme ? "☀️" : "🌙", "Toggle theme");
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    
    // Save/Load buttons
    QPushButton* saveButton = createIconButton("💾", "Save puzzle");
    QPushButton* loadButton = createIconButton("📂", "Load puzzle");
    
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveGrid);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadGrid);
    
    // Difficulty selector
    QComboBox* difficultyBox = new QComboBox(this);
    difficultyBox->addItems({"Easy", "Medium", "Hard", "Expert"});
    difficultyBox->setFixedWidth(120);
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
    
    rightControls->addWidget(themeButton);
    rightControls->addWidget(saveButton);
    rightControls->addWidget(loadButton);
    rightControls->addWidget(new QLabel("Difficulty:", this));
    rightControls->addWidget(difficultyBox);
    
    topSection->addWidget(titleLabel);
    topSection->addStretch();
    topSection->addLayout(rightControls);
    
    mainLayout->addLayout(topSection);
    
    // Stats section
    QHBoxLayout* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    timerLabel = new QLabel("00:00", this);
    mistakesLabel = new QLabel("Mistakes: 0/3", this);
    scoreLabel = new QLabel("Score: 0", this);
    statsLabel = new QLabel(this);
    
    QFont statsFont("SF Pro Display", 14);
    timerLabel->setFont(statsFont);
    mistakesLabel->setFont(statsFont);
    scoreLabel->setFont(statsFont);
    statsLabel->setFont(statsFont);
    
    updateStats();
    
    statsLayout->addWidget(timerLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(mistakesLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(scoreLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(statsLabel);
    
    mainLayout->addLayout(statsLayout);
    mainLayout->addSpacing(20);
    
    // Grid and controls section
    QHBoxLayout* gridAndControls = new QHBoxLayout();
    gridAndControls->setSpacing(30);
    
    // Create grid
    gridWidget = new SudokuGrid(this);
    connect(gridWidget, &SudokuGrid::gridChanged, this, &MainWindow::onGridChanged);
    connect(gridWidget, &SudokuGrid::validityChanged, this, &MainWindow::onValidityChanged);
    connect(gridWidget, &SudokuGrid::puzzleSolved, [this](int timeInSeconds) {
        gameTimer->stop();
        showMessage("🎉 Puzzle solved!", "#00b894");
        puzzlesSolved++;
        if (bestTime == -1 || timeInSeconds < bestTime) {
            bestTime = timeInSeconds;
        }
        settings.setValue("puzzlesSolved", puzzlesSolved);
        settings.setValue("bestTime", bestTime);
        updateStats();
    });
    
    // Create controls section
    QVBoxLayout* controlsLayout = new QVBoxLayout();
    controlsLayout->setSpacing(25);
    
    // Game controls
    QHBoxLayout* gameControls = new QHBoxLayout();
    gameControls->setSpacing(10);
    
    QPushButton* undoButton = createIconButton("↩️", "Undo");
    QPushButton* eraseButton = createIconButton("🗑️", "Erase");
    QPushButton* notesButton = createIconButton("✏️", "Notes mode");
    notesButton->setCheckable(true);
    QPushButton* hintButton = createIconButton("💡", "Get hint");
    
    connect(undoButton, &QPushButton::clicked, gridWidget, &SudokuGrid::undoMove);
    connect(eraseButton, &QPushButton::clicked, [this]() {
        if (gridWidget && gridWidget->getCurrentCell()) {
            gridWidget->getCurrentCell()->clear();
        }
    });
    connect(notesButton, &QPushButton::toggled, [this](bool checked) {
        if (gridWidget) {
            gridWidget->setNotesMode(checked);
        }
    });
    connect(hintButton, &QPushButton::clicked, gridWidget, &SudokuGrid::showHint);
    
    gameControls->addWidget(undoButton);
    gameControls->addWidget(eraseButton);
    gameControls->addWidget(notesButton);
    gameControls->addWidget(hintButton);
    gameControls->addStretch();
    
    // Number pad
    QGridLayout* numberPad = new QGridLayout();
    numberPad->setSpacing(10);
    for (int i = 0; i < 9; ++i) {
        QPushButton* numButton = createNumberButton(QString::number(i + 1));
        numberPad->addWidget(numButton, i / 3, i % 3);
    }
    
    // New game button
    QPushButton* newGameButton = new QPushButton("New Game", this);
    newGameButton->setFixedHeight(45);
    newGameButton->setFont(QFont("SF Pro Display", 16, QFont::DemiBold));
    newGameButton->setStyleSheet(getButtonStyle("#0984e3"));
    connect(newGameButton, &QPushButton::clicked, [this, difficultyBox]() {
        if (gridWidget) {
            gridWidget->newGame(difficultyBox->currentText());
            gameTimer->start(1000);
            timerLabel->setText("00:00");
            mistakes = 0;
            score = 0;
            mistakesLabel->setText("Mistakes: 0/3");
            scoreLabel->setText("Score: 0");
        }
    });
    
    controlsLayout->addLayout(gameControls);
    controlsLayout->addLayout(numberPad);
    controlsLayout->addWidget(newGameButton);
    controlsLayout->addStretch();
    
    gridAndControls->addWidget(gridWidget);
    gridAndControls->addLayout(controlsLayout);
    
    mainLayout->addLayout(gridAndControls);
    mainLayout->addSpacing(25);
    
    // Action buttons
    QHBoxLayout* actionButtons = new QHBoxLayout();
    actionButtons->setSpacing(15);
    
    loadExampleButton = createStyledButton("Load Example", "#ff9f43", "L");
    validateButton = createStyledButton("Validate", "#00b894", "V");
    solveButton = createStyledButton("Solve", "#0984e3", "S");
    clearButton = createStyledButton("Clear", "#d63031", "C");
    
    connect(loadExampleButton, &QPushButton::clicked, this, &MainWindow::onLoadExampleClicked);
    connect(validateButton, &QPushButton::clicked, this, &MainWindow::onValidateClicked);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    
    actionButtons->addWidget(loadExampleButton);
    actionButtons->addWidget(validateButton);
    actionButtons->addWidget(solveButton);
    actionButtons->addWidget(clearButton);
    
    mainLayout->addLayout(actionButtons);
    
    // Message label
    messageLabel = new QLabel(this);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setFont(QFont("SF Pro Display", 14));
    messageLabel->hide();
    mainLayout->addWidget(messageLabel);
} 