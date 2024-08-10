//
// Created by Gun woo Kim on 8/6/24.
//

#ifndef SUDOKUBOARD_H
#define SUDOKUBOARD_H

#include <iostream>
#include <vector>

class SudokuBoard {
private:
    std::vector<std::vector<char>> board;
    std::vector<std::vector<char>> board_solved;
    int steps;
    double time_spent;
    bool solved;
    // if board is put in `solve()` and can't find a solution, has_board becomes false.
    bool has_board;
    std::string algorithm_used;
public:
    SudokuBoard(std::vector<std::vector<char>> &&board_in)
        : board(std::move(board_in)), steps(0), solved(false), time_spent(0), algorithm_used("none"), has_board(true)
    {}
    SudokuBoard()
        : board(), steps(0), solved(false), time_spent(0), algorithm_used("none"), has_board(false) {
        board.resize(9, std::vector<char>(9, '.'));
    }

    // this gets board that can be modified directly.
    std::vector<std::vector<char>>& getOriginalBoard() {
        return board;
    }
    const std::vector<std::vector<char>>& getSolvedBoard() const {
        return board_solved;
    }
    int getSteps() const {
        return steps;
    }
    bool getSolvedStatus() const {
        return solved;
    }
    bool hasBoard() const {
        return has_board;
    }
    double getTime() const {
        return time_spent;
    }
    std::string getAlgorithmUsed() const {
        return algorithm_used;
    }


    // set board with move semantics
    void setOriginalBoard(std::vector<std::vector<char>>&& board_in) {
        board = std::move(board_in);
    }
    void setSolvedBoard(std::vector<std::vector<char>>&& board_in) {
        board_solved = std::move(board_in);
    }
    void setElapsedTime(double d) {
        time_spent = d;
    }
    void setAlgorithmUsed(std::string s) {
        algorithm_used = s;
    }
    void setHasBoard(bool b) {
        has_board = b;
    }
    // reverts everything as if it was not solved.
    void resetAll() {
        board = std::vector<std::vector<char>>();
        steps = 0;
        solved = false;
        time_spent = 0;
    }

    // prints a board in formatted way
    static void printBoard(const std::vector<std::vector<char>> &board) {
        if (board.empty()) return;

        using namespace std;
        for (int i = 0; i < 9; i++) {
            if (i % 3 == 0) {
                cout << "+-------+-------+-------+" << endl;
            }
            for (int j = 0; j < 9; j++) {
                if (j % 3 == 0) {
                    cout << "| ";
                }
                if (board[i][j] == '.')
                    cout << "  ";
                else
                    cout << board[i][j] << ' ';
                if (j == 8) {
                    cout << "|";
                }
            }
            cout << endl;
        }
        cout << "+-------+-------+-------+" << endl;
    }
    // prints the time spent to solve
    void printTime() const {
        if (algorithm_used == "none")
            std::cout << "Board unsolved yet." << std::endl;
        else
            std::cout << "Time Spent: " << time_spent << " seconds" << std::endl;
    }
    // prints which algorithm was used in solving the board
    void printAlgorithmUsed() const {
        if (algorithm_used == "none")
            std::cout << "Board unsolved yet." << std::endl;
        else
            std::cout << "Algorithm used: " << algorithm_used << std::endl;
    }
};

#endif //SUDOKUBOARD_H
