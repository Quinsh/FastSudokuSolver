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

    /**
     * solve should solve the board
     * also measures the execution time and save to the SudokuBoard and saves to SudokuBoard which algorithm was used.
     *
     * `solve` should be able to solve even if the board is changed, so need to call resourceClear() after solve.
     */
    virtual void solve(SudokuBoard &) = 0;

    /**
     * sets every variable to the state before calling solve(). Should be called after solve(), as solve can be called again with different board
     */
    virtual void resourceClear() = 0;

    void enableIterationCount() {iterationCount = true;}
    void disableIterationCount() {iterationCount = false;}

};

#endif //SUDOKUSOLVER_H
