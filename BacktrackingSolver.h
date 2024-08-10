//
// Created by Gun woo Kim on 8/6/24.
//

#ifndef BACKTRACKINGSOLVER_H
#define BACKTRACKINGSOLVER_H

#include <unordered_map>
#include <unordered_set>

#include "SudokuBoard.h"
#include "SudokuSolver.h"
#include "Timer.h"

class BacktrackingSolver : public SudokuSolver {

    std::unordered_map<int, std::unordered_set<char>> rowset, colset;
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<char>>> grid;
    bool found;  // Flag to indicate whether a solution has been found
    long long cnt;

public:
    BacktrackingSolver() : SudokuSolver(), found(false), cnt(0) {
    }

    virtual void solve(SudokuBoard &sb) override {
        rowset.clear(), colset.clear(), grid.clear();

        Timer t;
        std::vector<std::vector<char>> board = sb.getOriginalBoard();

        // Record the current state of the board
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j] != '.') {
                    rowset[i].insert(board[i][j]);
                    colset[j].insert(board[i][j]);
                    grid[i/3][j/3].insert(board[i][j]);
                }
            }
        }
        // Start backtracking
        found = false;
        backtrack(board, 0);
        if (found) { // if there was solution,
            // record time
            sb.setElapsedTime(t.end());
            // save answer
            sb.setSolvedBoard(std::move(board));
            sb.setAlgorithmUsed("basic backtracking");
        }
        else {
            // if couldn't solve the board, the board configuration was wrong, so we need to get another board.
            // set `hasBoard` as false for signal.
            sb.setHasBoard(false);
        }
    }

    void backtrack(std::vector<std::vector<char>>& board, int p) {
        // cnt++;

        int row = p / 9;
        int col = p % 9;

        if (p == 81) {  // Solution found
            found = true;
            return;
        }

        if (board[row][col] != '.') {  // Skip pre-filled cells
            backtrack(board, p + 1);
            return;
        }

        for (int i = 0; i < 9; i++) {
            char c = '1' + i;
            if (isRight(row, col, c)) {
                insert(row, col, c);
                board[row][col] = c;

                backtrack(board, p + 1);

                if (found) return;  // Early termination within the loop

                erase(row, col, c);
                board[row][col] = '.';
            }
        }

    }

    bool isRight(const int &row, const int &col, const char &x) {
        // Check if the character is already present in the row, column, or grid
        if (rowset[row].find(x) != rowset[row].end())
            return false;
        if (colset[col].find(x) != colset[col].end())
            return false;
        if (grid[row/3][col/3].find(x) != grid[row/3][col/3].end())
            return false;

        return true;
    }

    void insert(const int &i, const int &j, const char &x) {
        rowset[i].insert(x);
        colset[j].insert(x);
        grid[i/3][j/3].insert(x);
    }

    void erase(const int &i, const int &j, const char &x) {
        rowset[i].erase(x);
        colset[j].erase(x);
        grid[i/3][j/3].erase(x);
    }

};

#endif //BACKTRACKINGSOLVER_H
