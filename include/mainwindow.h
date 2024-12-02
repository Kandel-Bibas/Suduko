#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include "sudokugrid.h"
#include "solver.h"
#include <QTime>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onSolveClicked();
    void onClearClicked();
    void onLoadExampleClicked();
    void onValidateClicked();
    void onGridChanged();
    void onValidityChanged(bool isValid);
    void toggleTheme();
    void saveGrid();
    void loadGrid();
    void toggleFullscreen();
    void showStats();

private:
    void createMenus();
    void showMessage(const QString& message, const QString& color);
    QPushButton* createStyledButton(const QString& text, const QString& color, const QString& shortcut);
    QPushButton* createIconButton(const QString& icon, const QString& tooltip);
    QPushButton* createNumberButton(const QString& number);
    QString adjustColor(const QString& color, double factor);
    QString getButtonStyle(const QString& color);
    void applyTheme(bool isDark);
    void updateStats();
    
    SudokuGrid *gridWidget;
    QPushButton *solveButton;
    QPushButton *clearButton;
    QPushButton *validateButton;
    QPushButton *loadExampleButton;
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QLabel *messageLabel;
    QLabel *statsLabel;
    QLabel *timerLabel;
    QLabel *mistakesLabel;
    QLabel *scoreLabel;
    SudokuSolver solver;
    QSettings settings;
    bool isDarkTheme;
    int puzzlesSolved;
    int bestTime;
    QTime solveStartTime;
    int mistakes;
    int score;
    QTimer *gameTimer;
}; 