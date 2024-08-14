//
// Created by Gun woo Kim on 8/10/24.
//

#ifndef HEURISTICSBACKTRACKINGSOLVER_H
#define HEURISTICSBACKTRACKINGSOLVER_H

#include <queue>
#include <utility>

#include "BacktrackingSolver.h"
#include "SudokuSolver.h"

/* CONSIDERATIONS FOR EFFICIENCY IMPROVEMENT
 * NOTE: overall, I'm very unsatisfied with this code I have written. There are lots of places where I'm copying entire 2d vector or some map of set, in each recursion.
 * Also, there are O(1) TC functions with lots of lots of operations lol.. while coding, I took note of some things to fix:
 *
 * TODO: lookup for the cell with least candidates is unefficient as it iterates 81 times every time to find the min cell. find lazy way to do this?
 *  - I was thinking of using minheap but it's not good. As we need to construct minheap every time and that's O(N)
 * TODO: ruleBased returns an r-value. Instead of this, let's make it void fxn and change things directly with the passed parameter.
 *
 * TODO: don't erase or insert to every cell in row/col/grid. just insert to those that are empty. also, try not to repeat.
 */

/**
 * This approach combines Heuristics and Backtracking. We first find every possible cells with rule-based approach,
 * and then we make a guess and backtrack (see if it leads to wrong configuration when we recurse main fxn again with this guess).
 * Therefore this decreases number of unnecessary recursion for answers that can be figured out without guessing.
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
    std::unordered_map<int, std::unordered_set<char>> cellCandidates;

    // these are ONLY used in isSolved function for checking if the board is solved.
    // rowset/colset maps row/col index to corresponding set
    // gridset maps the index of left top corner of each grid to the set of that grid.
    // Ex: gridset[0] is valid(1st grid), gridset[30] is valid (this is the 5th grid).
    std::unordered_map<int, std::unordered_set<char>> rowset, colset, gridset;

public:
    // Function to print candidates for each cell in a formatted Sudoku board way
    void printCandidates() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                int cellIndex = row * 9 + col;
                if (cellCandidates.at(cellIndex).empty()) {
                    std::cout << " . ";  // Empty cell (no candidates)
                } else {
                    std::cout << "{";
                    for (const auto& candidate : cellCandidates.at(cellIndex)) {
                        std::cout << candidate;
                    }
                    std::cout << "} ";
                }

                // Print a divider after every 3 columns (for formatting)
                if (col == 2 || col == 5) {
                    std::cout << "| ";
                }
            }
            std::cout << std::endl;

            // Print a horizontal divider after every 3 rows (for formatting)
            if (row == 2 || row == 5) {
                std::cout << "---------+---------+---------" << std::endl;
            }
        }
    }

    /**
     * erases `n` from available number set of other cells in the same row, col, and grid.
     * TC: O(1), Θ(27)
     * @param row
     * @param col
     * @param n
     */
    void putNumber(const int &row, const int &col, const char &n) {
        // erase n from itself
        // cellCandidates[row*9+col].erase(n);
        cellCandidates[row*9+col].clear();
        // erase n from all cells in the same row
        for (int i = 0; i < 9; i++)
            cellCandidates[row*9+i].erase(n);
        // erase n from all cells in the same col
        for (int i = 0; i < 9; i++)
            cellCandidates[9*i+col].erase(n);
        // erase n from all cells in the same grid
        int grid_start_pos = (row/3)*3*9 + (col/3)*3; // this will find the "left upper cell" in the grid it's in.
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                cellCandidates[grid_start_pos+i*9+j].erase(n);
        }
    }

    /**
     * adds `n` to available number set of other cells in the same row, col, and grid.
     * TC: O(1), Θ(27)
     * @param row
     * @param col
     * @param n
     * @param candidateset
     */
    void eraseNumber(const int &row, const int &col, const char &n, std::unordered_set<char> candidateset) {
        // erase n from itself
        cellCandidates[row*9+col] = std::move(candidateset);
        // erase n from all cells in the same row
        for (int i = 0; i < 9; i++)
            cellCandidates[row*9+i].insert(n);
        // erase n from all cells in the same col
        for (int i = 0; i < 9; i++)
            cellCandidates[9*i+col].insert(n);
        // erase n from all cells in the same grid
        int grid_start_pos = (row/3)*3*9 + (col/3)*3; // this will find the "left upper cell" in the grid it's in.
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                cellCandidates[grid_start_pos+i*9+j].insert(n);
        }
    }

    /**
     * returns true if board is full and correct
     * TC: O(1), Θ(81)
     * @param board
     * @return bool
     */
    bool isSolved(std::vector<std::vector<char>>& board) {
        rowset.clear(), colset.clear(), gridset.clear(); // clear the set before using.
        int row, col, grid;
        char num;
        for (int i=0; i<81; i++) {
            num = board[i/9][i%9];
            row = i/9;
            col = i%9;
            grid = (row/3)*3*9 + (col/3)*3;
            if (num == '.') return false; // board incomplete
            if (rowset[row].find(num) == rowset[row].end() && colset[col].find(num) == colset[col].end() && gridset[grid].find(num) == gridset[grid].end()) {
                rowset[row].insert(num);
                colset[col].insert(num);
                gridset[grid].insert(num);
            }
            else {
                return false;
            }
        }
        return true;
    }

    /**
     * finds the empty cell with least number of candidates
     * TC: O(1), Θ(81)
     * @param board
     * @return index of cell with min candidate. 0 if there's not.
     */
    int findCellWithLeastCandidates(std::vector<std::vector<char>>& board) {
        int minCell = 0, minCandidates = INT_MAX;
        int sz;
        for (int i = 0; i < 81; i++) {
            sz = cellCandidates[i].size();
            if (board[i/9][i%9] == '.' && sz < minCandidates && sz != 0) {
                minCandidates = sz;
                minCell = i;
            }
        }
        return minCell;
    }

    /**
     * Apply the Naked Single Rule: if there is a cell with only 1 possible candidate, place it,
     * and erase it from candidate list of other cells in the same row/col/grid
     * TC: O(1), Θ(81 + 27 * number of naked single)
     * @param board
     * @return true if something was modified
     */
    bool applyNakedSingle(std::vector<std::vector<char>>& board) {
        bool flag = false;
        for (int i = 0; i < 81; i++) {
            if (board[i/9][i%9] == '.' && cellCandidates[i].size() == 1) {
                flag = true;
                auto num = *cellCandidates[i].begin();
                board[i/9][i%9] = num;
                putNumber(i/9, i%9, num);
            }
        }
        return flag;
    }

    // Custom hash function for unordered_set<char>
    struct UnorderedSetHash {
        std::size_t operator()(const std::unordered_set<char>& s) const {
            std::size_t hash = 0;
            for (const char& elem : s) {
                hash ^= std::hash<char>()(elem);
            }
            return hash;
        }
    };

    // Custom equality function for unordered_set<char>
    struct UnorderedSetEqual {
        bool operator()(const std::unordered_set<char>& s1, const std::unordered_set<char>& s2) const {
            return s1 == s2;
        }
    };

    /**
     * Naked Tuple Rule: if there are cells with naked tuple, those numbers can only be placed there, so erase
     * them from candidate list of other cells in the same row/col/grid.
     * Example, if a row is like ... | 2,3 | 2,3 | 2,3,7 | 2,3,8 | ... the first two are naked pair. the next two are hidden pair.
     * delete the hidden pair: ... | 2,3 | 2,3 | 7 | 8 | ...
     * TC: O(1).. but lot of operations happening here
     * @param board
     * @return true if something was modified
     *
     */
    bool applyNakedTuple(std::vector<std::vector<char>>& board) {
        bool flag = false;

        // process each row
        std::unordered_map<std::unordered_set<char>, std::unordered_set<int>, UnorderedSetHash, UnorderedSetEqual> nakedTupleRow;
        for (int row = 0; row < 9; row++) {
            nakedTupleRow.clear();
            for (int col = 0; col < 9; col++) {
                if (!cellCandidates[row*9+col].empty()) {
                    nakedTupleRow[cellCandidates[row*9+col]].insert(row*9+col);
                }
            }
            for (const auto& [numbers, indexes] : nakedTupleRow) {
                if (numbers.size() == indexes.size() && numbers.size() > 1) {
                    // then this is a naked tuple. Erase these numbers from the entire row except for `indexes`.
                    for (int col = 0; col < 9; col++) {
                        int cell = row*9+col;
                        if (indexes.find(cell) != indexes.end()) continue;
                        // erase the number in naked tuple from current cell's candidate set.
                        for (const auto &nt : numbers) {
                            if (cellCandidates[cell].erase(nt) > 0) flag = true;
                        }
                    }
                }
            }
        }

        // process each column
        std::unordered_map<std::unordered_set<char>, std::unordered_set<int>, UnorderedSetHash, UnorderedSetEqual> nakedTupleCol;
        for (int col = 0; col < 9; col++) {
            nakedTupleCol.clear();
            for (int row = 0; row < 9; row++) {
                if (!cellCandidates[row*9+col].empty()) {
                    nakedTupleCol[cellCandidates[row*9+col]].insert(row*9+col);
                }
            }
            for (const auto& [numbers, indexes] : nakedTupleCol) {
                if (numbers.size() == indexes.size() && numbers.size() > 1) {
                    // then this is a naked tuple. Erase these numbers from the entire col except for `indexes`.
                    for (int row = 0; row < 9; row++) {
                        int cell = row*9+col;
                        if (indexes.find(cell) != indexes.end()) continue;
                        // erase the number in naked tuple from current cell's candidate set.
                        for (const auto &nt : numbers) {
                            if (cellCandidates[cell].erase(nt) > 0) flag = true;
                        }
                    }
                }
            }
        }

        // process each grid
        std::unordered_map<std::unordered_set<char>, std::unordered_set<int>, UnorderedSetHash, UnorderedSetEqual> nakedTupleGrid;
        for (int gridrow = 0; gridrow < 3; gridrow++) {
            for (int gridcol = 0; gridcol < 3; gridcol++) {
                // process single grid.
                int gridstart = gridrow*27+gridcol*3;
                int gridend = gridstart+27;
                nakedTupleGrid.clear();
                for (int row = gridstart; row < gridend; row += 9) {
                    for (int index = row; index < row+3; index++) {
                        if (!cellCandidates[index].empty()) {
                            nakedTupleGrid[cellCandidates[index]].insert(index);
                        }
                    }
                }
                for (const auto& [numbers, indexes] : nakedTupleGrid) {
                    if (numbers.size() == indexes.size() && numbers.size() > 1) {
                        // then this is a naked tuple. Erase these numbers from the entire grid except for `indexes`.
                        for (int row = gridstart; row < gridend; row += 9) {
                            for (int cell = row; cell < row+3; cell++) {
                                if (indexes.find(cell) != indexes.end()) continue;
                                // erase the number in naked tuple from current cell's candidate set.
                                for (const auto &nt : numbers) {
                                    if (cellCandidates[cell].erase(nt) > 0) flag = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return flag;
    }

    /**
     * the chief function where `rule based` algorithm takes place.
     * @param board
     * @return some board. can be solution board or not.
     */
    std::vector<std::vector<char>> ruleBased(std::vector<std::vector<char>>& board) {
        while(true) {
            if (applyNakedSingle(board)) continue;
            if (applyNakedTuple(board)) continue;
            break;
        }

        // check if the board is solved only with heuristics.
        if (isSolved(board)) return board;

        // can't solve by heuristics at this point. do random guess, then backtrack with `ruleBased`

        int cell = findCellWithLeastCandidates(board); // conquer the cell with few candidates first.
        // save the current candidate
        auto cellCandidatesBackup = cellCandidates;
        auto boardBackup = board;
        auto candidates = cellCandidates[cell];
        for (const auto candidate : candidates) {
            // put number
            auto tempset = cellCandidates[cell];
            board[cell/9][cell%9] = candidate;
            putNumber(cell/9, cell%9, candidate);

            // recurse and check if returns a valid board
            std::vector<std::vector<char>> board_new = ruleBased(board);
            if (!board_new.empty() && isSolved(board_new))
                return board_new;

            // erase number to backtrack.
            board = boardBackup;
            cellCandidates = cellCandidatesBackup;
        }

        // couldn't find solution. Return empty board.
        return {};

    }

    virtual void solve(SudokuBoard& sb) override {
        resourceClear();
        Timer t;
        auto& board = sb.getOriginalBoard();

        // set the current configuration of the board.
        for (int i = 0; i <= 80; i++) {
            int row = i / 9, col = i % 9;
            if (board[row][col] == '.') continue;
            putNumber(row, col, board[row][col]);
        }
        // perform the rulebased backtracking algorithm:
        auto result = ruleBased(board);
        if (!result.empty()) {
            sb.setSolvedBoard(std::move(result));
            sb.setElapsedTime(t.end());
            sb.setAlgorithmUsed("heuristics backtracking");
        }
        else {
            // if couldn't solve the board, the board configuration was wrong, so we need to get another board.
            // set `hasBoard` as false for signal.
            sb.setHasBoard(false);
        }
    }

    virtual void resourceClear() override {
        for (int i = 0; i <= 80; i++) { // intialize every cell's set.
            cellCandidates[i] = std::unordered_set<char>{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
        }
    }


};


#endif //HEURISTICSBACKTRACKINGSOLVER_H
