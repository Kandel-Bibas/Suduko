#pragma once
#include <vector>
#include <memory>

class Node {
public:
    char character;
    bool isEndOfWord;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> middle;
    std::unique_ptr<Node> right;

    Node(char ch) : character(ch), isEndOfWord(false) {}
    Node(const Node& other) : character(other.character), isEndOfWord(other.isEndOfWord) {
        if (other.left) left = std::make_unique<Node>(*other.left);
        if (other.middle) middle = std::make_unique<Node>(*other.middle);
        if (other.right) right = std::make_unique<Node>(*other.right);
    }
};

class TernarySearchTree {
private:
    std::unique_ptr<Node> root;
    
    std::unique_ptr<Node> insert(std::unique_ptr<Node> node, char character) {
        if (!node) {
            return std::make_unique<Node>(character);
        }
        if (character < node->character) {
            node->left = insert(std::move(node->left), character);
        } else if (character > node->character) {
            node->right = insert(std::move(node->right), character);
        } else {
            node->isEndOfWord = true;
        }
        return node;
    }

    bool search(const Node* node, char character) const {
        if (!node) return false;
        if (character < node->character) return search(node->left.get(), character);
        if (character > node->character) return search(node->right.get(), character);
        return node->isEndOfWord;
    }

    std::unique_ptr<Node> deleteNode(std::unique_ptr<Node> node, char character) {
        if (!node) return nullptr;
        if (character < node->character) {
            node->left = deleteNode(std::move(node->left), character);
        } else if (character > node->character) {
            node->right = deleteNode(std::move(node->right), character);
        } else {
            node->isEndOfWord = false;
            if (!node->left && !node->middle && !node->right) {
                return nullptr;
            }
        }
        return node;
    }

public:
    TernarySearchTree() {
        for (char c = '1'; c <= '9'; ++c) {
            insert(c);
        }
    }

    TernarySearchTree(const TernarySearchTree& other) {
        if (other.root) {
            root = std::make_unique<Node>(*other.root);
        }
    }

    TernarySearchTree& operator=(const TernarySearchTree& other) {
        if (this != &other) {
            root.reset();
            if (other.root) {
                root = std::make_unique<Node>(*other.root);
            }
        }
        return *this;
    }

    void insert(char character) {
        root = insert(std::move(root), character);
    }

    bool search(char character) const {
        return search(root.get(), character);
    }

    void deleteCharacter(char character) {
        root = deleteNode(std::move(root), character);
    }

    bool isEmpty() const {
        return !root;
    }
};

class SudokuSolver {
private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<TernarySearchTree>> possibilities;

    void initializePossibilities() {
        possibilities.resize(9, std::vector<TernarySearchTree>(9));
    }

    bool isSafe(int row, int col, int num) {
        // Check row
        for (int x = 0; x < 9; x++)
            if (grid[row][x] == num)
                return false;

        // Check column
        for (int x = 0; x < 9; x++)
            if (grid[x][col] == num)
                return false;

        // Check 3x3 box
        int startRow = row - row % 3, startCol = col - col % 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (grid[i + startRow][j + startCol] == num)
                    return false;

        return true;
    }

    void propagateConstraints(int row, int col, int num) {
        char charNum = '0' + num;
        // Remove possibility from row
        for (int j = 0; j < 9; j++)
            possibilities[row][j].deleteCharacter(charNum);

        // Remove possibility from column
        for (int i = 0; i < 9; i++)
            possibilities[i][col].deleteCharacter(charNum);

        // Remove possibility from 3x3 box
        int boxRow = row - row % 3, boxCol = col - col % 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                possibilities[boxRow + i][boxCol + j].deleteCharacter(charNum);
    }

    bool solveSudoku() {
        int row = -1, col = -1;
        bool isEmpty = false;
        
        // Find empty cell
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) {
                    row = i;
                    col = j;
                    isEmpty = true;
                    break;
                }
            }
            if (isEmpty) break;
        }

        if (!isEmpty) return true;

        for (int num = 1; num <= 9; num++) {
            if (isSafe(row, col, num)) {
                grid[row][col] = num;
                propagateConstraints(row, col, num);

                if (solveSudoku())
                    return true;

                grid[row][col] = 0;
                initializePossibilities();
            }
        }
        return false;
    }

public:
    SudokuSolver(const std::vector<std::vector<int>>& board) : grid(board) {
        initializePossibilities();
    }

    bool solve() {
        return solveSudoku();
    }

    std::vector<std::vector<int>> getGrid() const { 
        return grid; 
    }

    void setGrid(const std::vector<std::vector<int>>& newGrid) { 
        grid = newGrid; 
        initializePossibilities();
    }
}; 