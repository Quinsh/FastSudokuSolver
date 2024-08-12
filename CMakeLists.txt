cmake_minimum_required(VERSION 3.10)
project(SudokuSolver)

set(CMAKE_CXX_STANDARD 17)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Tesseract
set(Tesseract_INCLUDE_DIRS "/opt/homebrew/opt/tesseract/include")
set(Tesseract_LIBRARIES "/opt/homebrew/opt/tesseract/lib/libtesseract.dylib")

include_directories(${Tesseract_INCLUDE_DIRS})

# Create the executable
add_executable(SudokuSolver main.cpp
        SudokuBoard.h
        SudokuSolver.h
        BacktrackingSolver.h
        Timer.h
        HeuristicsBacktrackingSolver.h
)

# Link OpenCV libraries
target_link_libraries(SudokuSolver ${OpenCV_LIBS} ${Tesseract_LIBRARIES})