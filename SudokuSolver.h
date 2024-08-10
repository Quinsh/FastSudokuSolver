//
// Created by Gun woo Kim on 8/6/24.
//

#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

#include "SudokuBoard.h"
#include <vector>

class SudokuSolver {
protected:
    bool iterationCount;
public:
    SudokuSolver()
        : iterationCount(false) {
    }
    virtual ~SudokuSolver() = default;

    /*
     * solve should solve the board
     * also measures the execution time and save to the SudokuBoard,
     * also saves to SudokuBoard which algorithm was used.
     */
    virtual void solve(SudokuBoard &) = 0;

    void enableIterationCount() {iterationCount = true;}
    void disableIterationCount() {iterationCount = false;}

};

#endif //SUDOKUSOLVER_H
