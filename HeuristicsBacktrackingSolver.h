//
// Created by Gun woo Kim on 8/10/24.
//

#ifndef HEURISTICSBACKTRACKINGSOLVER_H
#define HEURISTICSBACKTRACKINGSOLVER_H

#include <queue>

#include "BacktrackingSolver.h"
#include "SudokuSolver.h"

/* CONSIDERATIONS FOR EFFICIENCY IMPROVEMENT
 *
 * TODO: lookup for the cell with least candidates is unefficient as it iterates 81 times every time to find the min cell. find lazy way to do this?
 *  - I was thinking of using minheap but it's not good. As we need to construct minheap every time and that's O(N)
 * TODO: ruleBased returns a vector. Instead of this, let's make it void ruleBased and change things directly with the passed parameter.
 *
 * TODO:
 */

/**
 * This approach combines Heuristics and Backtracking. We first find every possible cells with rule-based approach,
 * and then we make a guess and backtrack (see if it leads to wrong configuration when we recurse main fxn again with this guess).
 * Therefore this decreases number of unnecessary recursion for obvious answers.
 *
 * Rule-based approach is as follows:
 * we make use of the already popular sudoku solving trick of Naked Tuples ("Naked Single", "Naked Pairs", and so on..) and
 * Hidden Tuples ("Hidden Single", "Hidden Pairs", and so on...). Notice that naked tuples and hidden tuples has some inverse relationship:
 * if 2 cells out of 5 are naked pairs, the remaining are hidden triplets (Berggren, P., & Nilsson, D. Page 14).
 *
 * The algorithm pseudocode (modified from Berggren, P., & Nilsson, D. Page 15) would be:
 *
 * puzzle ruleBased(puzzle):
 *   while (true) { // while we can figure things out with heuristics
 *     if (applyNakedSingle(board))
 *       continue
 *     if (applyNakedTuple(board))
 *       continue
 *      break;
 *   }
 *
 *   if isSolved(puzzle)
 *     return puzzle
 *
 *   // guessing part
 *   x, y = findCellWithLeastCandidates(puzzle)
 *   for i in puzzle[y][x].canditates():
 *     puzzle[y][x] = i // assign;
 *     puzzle' = ruleBased(puzzle) // if this returns something, we found a solution board.
 *     if (puzzle')
 *       return puzzle'
 *
 *   return null; // if none of guesses lead to some valid board solution, this configuration doesn't have answer.
 */
class HeuristicsBacktrackingSolver : public SudokuSolver {
    // mapping of cell 1-81 to available numbers to put in that cell
    // this should be updated everytime a number is put or erased.
    std::unordered_map<int, std::unordered_set<char>> availableNums;



public:

    // erases `n` from available number set of other cells in the same row, col, and grid.
    void putNumber(const int &row, const int &col, const char &n) {
        // erase n from itself
        availableNums[row*9+col].erase(n);
        // erase n from all cells in the same row
        for (int i = 0; i < 9; i++)
            availableNums[row*9+i].erase(n);
        // erase n from all cells in the same col
        for (int i = 0; i < 9; i++)
            availableNums[9*i+col].erase(n);
        // erase n from all cells in the same grid
        int grid_start_pos = (row/3)*3*9 + (col/3)*3; // this will find the "left upper cell" in the grid it's in.
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                availableNums[grid_start_pos+i*9+j].erase(n);
        }
    }

    // adds `n` to available number set of other cells in the same row, col, and grid.
    void eraseNumber(const int &row, const int &col, const char &n) {
        // erase n from itself
        availableNums[row*9+col].insert(n);
        // erase n from all cells in the same row
        for (int i = 0; i < 9; i++)
            availableNums[row*9+i].insert(n);
        // erase n from all cells in the same col
        for (int i = 0; i < 9; i++)
            availableNums[9*i+col].insert(n);
        // erase n from all cells in the same grid
        int grid_start_pos = (row/3)*3*9 + (col/3)*3; // this will find the "left upper cell" in the grid it's in.
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                availableNums[grid_start_pos+i*9+j].insert(n);
        }
    }

    // returns true if board is full and correct
    bool isSolved(std::vector<std::vector<char>>& board) {

    }

    // finds the empty cell with least candidates with the class variable `availableNums`
    int findCellWithLeastCandidates(std::vector<std::vector<char>>& board) {
        int minCell = 0, minCandidates = INT_MAX;
        int sz;
        for (int i = 0; i < 81; i++) {
            sz = availableNums[i].size();
            if (board[i/9][i%9] == '.' && sz < minCandidates) {
                minCandidates = sz;
                minCell = i;
            }
        }
        return minCell;
    }


    bool applyNakedSingle(std::vector<std::vector<char>>& board) {

    }

    bool applyNakedTuple(std::vector<std::vector<char>>& board) {

    }

    /**
     *
     * @param board
     * @return some board. can be solution board or not.
     */
    std::vector<std::vector<char>> ruleBased(std::vector<std::vector<char>>& board) {
        while(true) {
            if (applyNakedSingle(board)) continue;
            if (applyNakedTuple(board)) continue;
            break;
        }

        // can't solve by heuristics at this point. do random guess, then backtrack with `ruleBased`
        int cell = findCellWithLeastCandidates(board); // conquer the cell with few candidates first.
        for (const auto candidate : availableNums[cell]) {
            // put number
            board[cell/9][cell%9] = candidate;
            putNumber(cell/9, cell%9, candidate);

            // recurse and check if returns a valid board
            std::vector<std::vector<char>> board_new = ruleBased(board);
            if (isSolved(board_new))
                return board_new;

            // erase number to backtrack.
            board[cell/9][cell%9] = '.';
            eraseNumber(cell/9, cell%9, candidate);
        }

        // couldn't find solution. Return empty board.
        return {};

    }

    virtual void solve(SudokuBoard& sb) override {
        resourceClear();
        auto& board = sb.getOriginalBoard();

        // set the current configuration of the board.
        for (int i = 0; i <= 80; i++) {
            int row = i / 9, col = i % 9;
            if (board[row][col] == '.') continue;
            putNumber(row, col, board[row][col]);
        }
        // perform the rulebased backtracking algorithm:
        ruleBased(board);
    }

    virtual void resourceClear() {
        for (int i = 0; i <= 80; i++) { // intialize every cell's set.
            availableNums[i] = std::unordered_set<char>{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
        }
    }


};


#endif //HEURISTICSBACKTRACKINGSOLVER_H
