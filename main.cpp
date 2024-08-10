#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <tesseract/baseapi.h>

#include "BacktrackingSolver.h"
#include "SudokuSolver.h"
#include "SudokuBoard.h"

#define BOARD_WIDTH 495;
#define BOARD_HEIGHT 495; // choose 495x495 because it divides wholly into 9

using namespace std;

// int main() {
//     vector<vector<char>> board = {
//         {'5', '3', '.', '.', '7', '.', '.', '.', '.'},
//         {'6', '.', '.', '1', '9', '5', '.', '.', '.'},
//         {'.', '9', '8', '.', '.', '.', '.', '6', '.'},
//         {'8', '.', '.', '.', '6', '.', '.', '.', '3'},
//         {'4', '.', '.', '8', '.', '3', '.', '.', '1'},
//         {'7', '.', '.', '.', '2', '.', '.', '.', '6'},
//         {'.', '6', '.', '.', '.', '.', '2', '8', '.'},
//         {'.', '.', '.', '4', '1', '9', '.', '.', '5'},
//         {'.', '.', '.', '.', '8', '.', '.', '7', '9'}
//     };
//
//     SudokuBoard sb1(std::move(board));
//
//     BacktrackingSolver solver_backtracking;
//     SudokuSolver &solver = solver_backtracking;
//
//     solver.solve(sb1);
//
//     // print solved status
//     SudokuBoard::printBoard(sb1.getSolvedBoard());
//     sb1.printTime();
//     sb1.printAlgorithmUsed();
//
//     return 0;
// }

// convers raw image to canny (just to look more cool)
void to_canny(cv::Mat& in, cv::Mat& out) {
    cv::Mat grayed, blurred;
    // convert to gray
    cv::cvtColor(in, grayed, cv::COLOR_BGR2GRAY);
    // apply gaussian blur
    cv::GaussianBlur(grayed, blurred, cv::Size(5, 5), 1);
    // canny
    cv::Canny(blurred, out, 50, 150);
    // convert to 3channel
    cv::cvtColor(out, out, cv::COLOR_GRAY2BGRA);
}

// convert raw image (colored) to grayscale, thresholded, noise reduced image
void img_preprocess(cv::Mat& in, cv::Mat& out) {
    cv::Mat grayed, blurred, thresholded, morph;
    // convert to gray
    cv::cvtColor(in, grayed, cv::COLOR_BGR2GRAY);
    // apply gaussian blur
    cv::GaussianBlur(grayed, blurred, cv::Size(5, 5), 1);
    // canny
    // cv::Canny(blurred, out, 50, 150);
    // threshold
    cv::adaptiveThreshold(blurred, thresholded, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 11, 2);
    // morphologiacl operation
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(thresholded, morph, cv::MORPH_CLOSE, kernel);
    // median blur to reduce salt-and-pepper noise
    cv::medianBlur(morph, out, 3);
}

// finds the coordinates of the (supposedly) sudoku board
std::vector<cv::Point> findBoardCoordinates(cv::Mat& in, cv::Mat& img_to_draw) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(in, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0, tempArea;
    std::vector<cv::Point> maxContour;

    for (const auto contour : contours) {
        // approximate the contour with simpler lines
        std::vector<cv::Point> quadApprox;
        cv::approxPolyDP(contour, quadApprox, cv::arcLength(contour, true) * 0.02, true);

        tempArea = cv::contourArea(quadApprox);

        // if the Area is too small, continue
        if (tempArea < 10000) continue; // 1000=30x30 pixel spotted

        // if it's not rectangular contour, continue
        if (quadApprox.size() != 4) continue;

        // if it's not approximately square, continue
        double side_len1 = 0, side_len2 = 0;
        double epsilon = 0.78; // this is the minimum ratio of smallerside/biggerside allowed (1 would be a perfect rectangle)
        for (int i = 1; i < 4; i++) {
            side_len1 = cv::norm(quadApprox[i] - quadApprox[(i+1) % 4]); // norm calculates the eucliean dist btw 2points
            side_len2 = cv::norm(quadApprox[(i+1) % 4] - quadApprox[(i+2) % 4]);
            double greater_side = max(side_len1, side_len2);
            double smaller_side = min(side_len1, side_len2);
            if ((smaller_side/greater_side) < epsilon) goto nextcontour;
        }

        if (tempArea > maxArea) {
            maxArea = tempArea;
            maxContour = quadApprox;
        }

        nextcontour:
    }
    // visualize the detected
    cv::polylines(img_to_draw, maxContour, true, cv::Scalar(0, 0, 255), 3);
    return maxContour;
}

// Store the warped Sudoku board image
void warpImage(const std::vector<cv::Point>& coords, const cv::Mat& img_src, cv::Mat& img_output) {
    float w = BOARD_WIDTH;
    float h = BOARD_HEIGHT; // 495 cuz this is perfectly divided into 9.
    cv::Point2f src_points[4];
    copy(coords.begin(), coords.end(), src_points);
    cv::Point2f dest_points[4] = {{w, 0}, {0, 0}, {0, h}, {w, h}}; // TODO: idk why but top right corner is at index 0 in src_points, so I had to

    cv::Mat transform_mat = cv::getPerspectiveTransform(src_points, dest_points);
    cv::warpPerspective(img_src, img_output, transform_mat, cv::Point(w, h));
}

// given a square cell of number image, extract the box contour of the number.
cv::Mat extractDigit(cv::Mat &img_cell) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(img_cell, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return img_cell; // Return the original cell if contour not found
    }

    // Find the largest Contour
    int largestContourIndex = -1;
    double largestContourArea = 0;
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > largestContourArea) {
            largestContourArea = area;
            largestContourIndex = i;
        }
    }

    if (largestContourIndex == -1) {
        return img_cell; // Return the original cell if no valid contours are found
    }

    // Extract the bounding box of the largest contour
    cv::Rect boundingBox = cv::boundingRect(contours[largestContourIndex]);
    cv::Mat digitROI = img_cell(boundingBox);

    // Resize the extracted digit to a standard size for better OCR accuracy
    // cv::Mat resizedDigit;
    // cv::resize(digitROI, resizedDigit, cv::Size(28, 28));

    return digitROI;
}

// calculates the percentage of black pixels in some gray scale cv::mat. A threshold is applied to identify "black"
double blackPixelPercentage(cv::Mat& gray) {

    cv::Mat blackMask;
    cv::threshold(gray, blackMask, 15, 255, cv::THRESH_BINARY_INV);

    int blackPixelCount = cv::countNonZero(blackMask);

    int totalPixels = gray.rows * gray.cols;

   return (static_cast<double>(blackPixelCount) / totalPixels) * 100.0;
}

// builds the square img of solution to the board (not warped yet)
cv::Mat buildSolutionImage(SudokuBoard &sb) {
    if (sb.getSolvedBoard().empty()) return cv::Mat();
    int w = BOARD_WIDTH; int h = BOARD_HEIGHT;
    cv::Mat img_solution = cv::Mat::zeros(w, h, CV_8UC4);
    const vector<vector<char>>& board_unsolved = sb.getOriginalBoard();
    const vector<vector<char>>& board_solved = sb.getSolvedBoard();
    int x, y;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board_unsolved[i][j] == '.') {
                x = w/9 * j + w/25, y = h/9 * i + h/12;

                cv::putText(img_solution, to_string(board_solved[i][j] - '0'), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1.1, cv::Scalar(0, 255, 0), 2.5);
            }
        }
    }
    cv::imshow("Solution Image", img_solution);
    return img_solution;
}

//
void printMetrics(SudokuBoard& sb, cv::Mat& img_canvas) {
    int w = img_canvas.cols;
    int h = img_canvas.rows;

    cv::putText(img_canvas, "Algorithm: " + sb.getAlgorithmUsed(), cv::Point(2 * w/5, h - 2 * h/16 - h/32), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 3.5);
    cv::putText(img_canvas, "Time Spent: " + to_string(sb.getTime()) + " seconds", cv::Point(2 * w/5, h - h/16), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 3.5);
}

// warp and overlay the solution image to the background
void warpSolutionImage(const std::vector<cv::Point>& coords, cv::Mat &img_sol, cv::Mat& img_canvas) {
    float w = BOARD_WIDTH;
    float h = BOARD_HEIGHT;
    cv::Point2f src_points[4] = {{w, 0}, {0, 0}, {0, h}, {w, h}};
    cv::Point2f dest_points[4];
    std::copy(coords.begin(), coords.end(), dest_points);

    cv::Mat transform_mat = cv::getPerspectiveTransform(src_points, dest_points);

    // Warping
    cv::Mat img_warped;
    cv::warpPerspective(img_sol, img_warped, transform_mat, img_canvas.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    // mask where the non-black pixels of the warped image are set
    cv::Mat mask;
    cv::cvtColor(img_warped, mask, cv::COLOR_BGR2GRAY);
    cv::threshold(mask, mask, 1, 255, cv::THRESH_BINARY);

    // copy only the non-black part of img_warped to img_canvas
    img_warped.copyTo(img_canvas, mask);
}

bool parseSudokuBoard(SudokuBoard& sb, cv::Mat& img_sudoku) {
    tesseract::TessBaseAPI tess;
    if (tess.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY)) {
        std::cerr << "Could not initialize tesseract.\n";
        return false;
    }
    tess.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);

    int width = img_sudoku.cols, height = img_sudoku.rows;
    int cell_sz = width / 9;
    int line_sz = cell_sz / 7; // we consider the line to be 1/7 of cell size

    auto &board = sb.getOriginalBoard();
    // Rotate the matrix in every direction.
    cv::Mat img_rotated = img_sudoku.clone();
    unsigned int maxParseCnt = 0;

    for (int rotateCode = -1; rotateCode < 3; rotateCode++) {
        if (rotateCode != -1)
            cv::rotate(img_sudoku, img_rotated, rotateCode);

        unsigned int parsedCount = 0;
        vector<vector<char>> tempBoard; tempBoard.resize(9, vector<char>(9, '.'));

        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                cv::Rect roi(j * cell_sz + line_sz, i * cell_sz + line_sz, cell_sz - 2 * line_sz, cell_sz - 2 * line_sz);
                cv::Mat img_cell = img_rotated(roi);

                // if img_cell is almost 88% composed of black, just skip it, as it's not a character
                if (blackPixelPercentage(img_cell) > 88.0)
                    continue;

                // Preprocess each cell for better OCR accuracy
                cv::Mat img_cell_processed = img_cell; // extractDigit(img_cell);

                // cv::imshow("row " + to_string(i) + " col: " + to_string(j), img_cell);

                // Parse img_cell with tesseract
                tess.SetImage(img_cell_processed.data, img_cell_processed.cols, img_cell_processed.rows, 1, img_cell_processed.step);
                tess.Recognize(0);
                tesseract::ResultIterator* ri = tess.GetIterator();
                if (ri != nullptr) {
                    do {
                        const char* symbol = ri->GetUTF8Text(tesseract::RIL_SYMBOL);
                        float conf = ri->Confidence(tesseract::RIL_SYMBOL);

                        if (symbol != nullptr && std::isdigit(symbol[0]) && conf > 95.0) { // Only accept high confidence digits
                            tempBoard[i][j] = symbol[0]; // Convert char to int
                            parsedCount++;
                        }

                        delete[] symbol;
                    } while (ri->Next(tesseract::RIL_SYMBOL));
                    delete ri;
                }
            }
        }

        if (parsedCount > maxParseCnt) { // if this version of rotation could parse more things than the max so far, use this one
            sb.setOriginalBoard(std::move(tempBoard));
            maxParseCnt = parsedCount;
        }
    }

    tess.End();
    if (maxParseCnt > 9) { // if we can parse more than 9 cells, we consider it as correct board parsing.
        sb.setHasBoard(true);
        return true;
    }
    return false;
}

int main() {
    using namespace cv;
    Mat img, img_processed;
    SudokuBoard sb;
    BacktrackingSolver solver_backtracking; // change here whatever solver you want to use.
    SudokuSolver &solver = solver_backtracking;
    cv::Mat img_solution;

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video capture device." << std::endl;
        return -1;
    }

    while (true) {
        cap.read(img);
        if (img.empty()) {
            std::cerr << "Error: Could not read frame from video capture device." << std::endl;
            break;
        }

        // make a screen to show (apply canny to look cool!)
        Mat img_show;
        to_canny(img, img_show);

        // Process image and store in img_processed
        img_preprocess(img, img_processed);

        // Find coordinates and draw the red contour
        std::vector<Point> coords = findBoardCoordinates(img_processed, img_show);

        if (!sb.hasBoard()) { // if board was not parsed yet, (if board is parsed, sb.hasBoard changes to true)

            Mat img_sudoku;
            warpImage(coords, img_processed, img_sudoku);
            imshow("Sudoku Board Warped", img_sudoku);

            // if coords is not empty, parse the sudokuBoard.
            if (!coords.empty()) {
                parseSudokuBoard(sb, img_sudoku); // this return bool (parsed correctly or not), but we will be using sb.hasBoard()
            }

            // print board TODO: this is for testing, erase later
            // SudokuBoard::printBoard(sb.getOriginalBoard());


            if (sb.hasBoard()) { // if it parsed board, solve the answer (notice this will only run once)
                solver.solve(sb); // solve() will set sb.hasBoard() to false if can't solve, bcz that means the board has wrong configuration. It will look for another board.
                SudokuBoard::printBoard(sb.getSolvedBoard());
                img_solution = buildSolutionImage(sb);
            }
        }

        if (sb.hasBoard() && !coords.empty()) {
            warpSolutionImage(coords, img_solution, img_show);
            printMetrics(sb, img_show);
        }
        imshow("camera", img_show);

        // if (waitKey(50) >= 0) break;
        waitKey(100);
    }


    return 0;
}
