#include <iostream>
#include <vector>
using namespace std;

#saleep

class Node
{
public:
    char character;
    bool isEndOfWord;
    Node *left;
    Node *middle;
    Node *right;

    Node(char ch) : character(ch), isEndOfWord(false), left(nullptr), middle(nullptr), right(nullptr) {}
};

class TernarySearchTree
{
private:
    Node *root;

    Node *insert(Node *node, char character)
    {
        if (!node)
            node = new Node(character);

        if (character < node->character)
            node->left = insert(node->left, character);
        else if (character > node->character)
            node->right = insert(node->right, character);
        else
            node->isEndOfWord = true;

        return node;
    }

    bool search(Node *node, char character) const
    {
        if (!node)
            return false;

        if (character < node->character)
            return search(node->left, character);
        else if (character > node->character)
            return search(node->right, character);
        else
            return node->isEndOfWord;
    }

    Node *deleteNode(Node *node, char character)
    {
        if (!node)
            return nullptr;

        if (character < node->character)
            node->left = deleteNode(node->left, character);
        else if (character > node->character)
            node->right = deleteNode(node->right, character);
        else
        {
            node->isEndOfWord = false;
            if (!node->left && !node->middle && !node->right)
            {
                delete node;
                return nullptr;
            }
        }
        return node;
    }

public:
    TernarySearchTree() : root(nullptr)
    {
        for (char c = '1'; c <= '9'; ++c)
        {
            insert(c);
        }
    }

    void insert(char character)
    {
        root = insert(root, character);
    }

    bool search(char character) const
    {
        return search(root, character);
    }

    void deleteCharacter(char character)
    {
        root = deleteNode(root, character);
    }

    bool isEmpty() const
    {
        return root == nullptr;
    }
};

class SudokuSolver
{
private:
    vector<vector<int>> grid;
    vector<vector<TernarySearchTree>> possibilities;

    void initializePossibilities()
    {
        possibilities = vector<vector<TernarySearchTree>>(9, vector<TernarySearchTree>(9));
    }

    bool isSafe(int row, int col, int num)
    {
        char charNum = '0' + num;

        for (int i = 0; i < 9; ++i)
        {
            if (grid[row][i] == num || grid[i][col] == num)
                return false;
            if (grid[3 * (row / 3) + i / 3][3 * (col / 3) + i % 3] == num)
                return false;
        }
        return true;
    }

    void propagateConstraints(int row, int col, int num)
    {
        char charNum = '0' + num;

        for (int i = 0; i < 9; ++i)
        {
            possibilities[row][i].deleteCharacter(charNum);
            possibilities[i][col].deleteCharacter(charNum);
            possibilities[3 * (row / 3) + i / 3][3 * (col / 3) + i % 3].deleteCharacter(charNum);
        }
    }

    bool solveSudoku()
    {
        int row = -1, col = -1;
        bool isEmpty = true;
        for (int i = 0; i < 9 && isEmpty; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (grid[i][j] == 0)
                {
                    row = i;
                    col = j;
                    isEmpty = false;
                    break;
                }
            }
        }

        if (isEmpty)
            return true;

        for (char num = '1'; num <= '9'; ++num)
        {
            int intNum = num - '0';
            if (isSafe(row, col, intNum))
            {
                grid[row][col] = intNum;
                propagateConstraints(row, col, intNum);

                if (solveSudoku())
                    return true;

                grid[row][col] = 0;
                initializePossibilities();
            }
        }
        return false;
    }

public:
    SudokuSolver(const vector<vector<int>> &board) : grid(board)
    {
        initializePossibilities();
    }

    void solve()
    {
        if (solveSudoku())
            cout << "Sudoku solved successfully!" << endl;
        else
            cout << "No solution exists for the Sudoku puzzle!" << endl;
    }

    void printGrid() const
    {
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                cout << grid[i][j] << " ";
            }
            cout << endl;
        }
    }
};

int main()
{
    vector<vector<int>> board(9, vector<int>(9, 0));

    int initialBoard[9][9] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}};

    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j)
            board[i][j] = initialBoard[i][j];

    SudokuSolver solver(board);

    cout << "Original Sudoku Puzzle:" << endl;
    solver.printGrid();
    cout << endl;

    solver.solve();

    cout << "\nSolved Sudoku Puzzle:" << endl;
    solver.printGrid();

    return 0;
}
