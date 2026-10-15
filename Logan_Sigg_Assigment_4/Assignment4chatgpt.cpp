/*
 * Program: EECS 348 Assignment 4
 *
 * Description:
 * This C++ program reads and solves five Sudoku puzzles
 * The program uses recursive depth-first search with backtracking
 * to find every valid solution
 * Each puzzle is printed before its solutions
 * Puzzles without a valid solution display "No solution found"
 *
 * Inputs:
 * puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, and puzzle5.txt
 * Each file contains a 9-by-9 puzzle with digits 1 through 9 and underscores for blanks
 *
 * Outputs:
 * The file name and original puzzle for each input file
 * Every valid solution or a message when no solution is found
 *
 * Collaborators:
 * None
 *
 * Other Sources:
 * ChatGPT was used to generate the initial C++ implementation
 * The assignment instructions provided the program requirements
 *
 * Author: Logan Sigg
 * Creation Date: 10/11/2026
 * Revision Date: 10/14/2026
 *
 */

/* Import the standard facilities used for files, strings, arrays, and output */
/* Store the 9-by-9 board with fixed-size arrays */
#include <array>

/* Read the five supplied text files */
#include <fstream>

/* Print puzzles, solutions, and errors */
#include <iostream>

/* Build file names and error descriptions */
#include <string>

/* Define one object that owns its puzzle board and search state */
class SudokuSolver {
/* These members are used only within the class */
private:

    /* Zero means an empty square */
    std::array<std::array<int, 9>, 9> board{};

    /* Count solutions for this puzzle */
    unsigned long long solutionCount = 0;

    /* Check whether a digit is absent from its row, column, and 3-by-3 box */
    bool canPlace(int row, int col, int digit) const {

        /* Inspect every row/column position */
        for (int i = 0; i < 9; ++i) {
            /* Reject a digit already used in the current row or column */
            if (board[row][i] == digit || board[i][col] == digit) {

                /* The digit already exists in this row/column */
                return false;
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* Find the box's first row */
        const int boxRow = (row / 3) * 3;

        /* Find the box's first column */
        const int boxCol = (col / 3) * 3;

        /* Inspect box rows */
        for (int r = boxRow; r < boxRow + 3; ++r) {

            /* Inspect box cells */
            for (int c = boxCol; c < boxCol + 3; ++c) {

                /* Reject a duplicate in the box */
                if (board[r][c] == digit) {
                    /* Return false to report a conflicting digit or invalid puzzle */
                    return false;
                /* End this block and return to the enclosing scope */
                }
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* No Sudoku constraint would be violated */
        return true;
    /* End this block and return to the enclosing scope */
    }

    /* Display the current board, showing underscores for empty squares */
    void printBoard() const {

        /* Print each board row */
        for (int row = 0; row < 9; ++row) {

            /* Print each cell in that row */
            for (int col = 0; col < 9; ++col) {

                /* Separate cells with spaces */
                if (col != 0) std::cout << ' ';

                /* Print an underscore for an empty cell */
                if (board[row][col] == 0) std::cout << '_';

                /* Fixed or solved digit */
                else std::cout << board[row][col];
            /* End this block and return to the enclosing scope */
            }

            /* End the current row */
            std::cout << '\n';
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

    /* Recursively enumerate every valid completion of the current board */
    void search() {

        /* Row of the next square to fill */
        int bestRow = -1;

        /* Column of the next square to fill */
        int bestCol = -1;

        /* Candidate digits for the selected square */
        int bestMask = 0;

        /* Candidate count; at most nine are possible */
        int fewest = 10;

        /* Examine every board row */
        for (int row = 0; row < 9; ++row) {

            /* Examine every board cell */
            for (int col = 0; col < 9; ++col) {

                /* Skip filled cells */
                if (board[row][col] != 0) continue;

                /* Track permitted digits with nine bits */
                int mask = 0;

                /* Count candidates for this empty cell */
                int count = 0;

                /* Try each digit */
                for (int digit = 1; digit <= 9; ++digit) {

                    /* Check all constraints */
                    if (canPlace(row, col, digit)) {

                        /* Mark this digit as available */
                        mask |= (1 << digit);

                        /* Record another candidate */
                        ++count;
                    /* End this block and return to the enclosing scope */
                    }
                /* End this block and return to the enclosing scope */
                }

                /* Dead end: backtrack immediately */
                if (count == 0) return;

                /* Prefer the most constrained cell */
                if (count < fewest) {

                    /* Save its candidate count */
                    fewest = count;

                    /* Save its row index */
                    bestRow = row;

                    /* Save its column index */
                    bestCol = col;

                    /* Save its available digits */
                    bestMask = mask;
                /* End this block and return to the enclosing scope */
                }
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* No empty cells remain: found a solution */
        if (bestRow == -1) {

            /* Number the solution */
            ++solutionCount;

            /* Print the solution number */
            std::cout << "Solution " << solutionCount << ":\n";

            /* Print the completed valid board */
            printBoard();

            /* Separate multiple solutions */
            std::cout << '\n';

            /* Continue searching through earlier recursive calls */
            return;
        /* End this block and return to the enclosing scope */
        }

        /* Explore candidates */
        for (int digit = 1; digit <= 9; ++digit) {

            /* Skip digits that are not available for this cell */
            if ((bestMask & (1 << digit)) == 0) continue;

            /* Make a tentative choice */
            board[bestRow][bestCol] = digit;

            /* Recursively complete the remaining empty cells */
            search();

            /* Undo choice to find other solutions */
            board[bestRow][bestCol] = 0;
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

/* These members can be accessed outside the class */
public:
    /* Read exactly 81 whitespace-separated characters from a puzzle file */
    bool load(const std::string& filename) {

        /* Open the named puzzle file */
        std::ifstream input(filename);

        /* Detect a missing or inaccessible file */
        if (!input) {

            /* Report that the puzzle file could not be opened */
            std::cerr << "Unable to open " << filename << '\n';

            /* Report failure to the caller */
            return false;
        /* End this block and return to the enclosing scope */
        }

        /* Clear the board before loading a new puzzle */
        board = {};

        /* Reset solution count for this puzzle */
        solutionCount = 0;

        /* Read nine puzzle rows */
        for (int row = 0; row < 9; ++row) {

            /* Read nine cells per row */
            for (int col = 0; col < 9; ++col) {

                /* Hold the next input character */
                char ch;

                /* Ensure all 81 cells are present */
                if (!(input >> ch)) {
                    /* Print the file name when fewer than 81 cells are provided */
                    std::cerr << "Incomplete puzzle: " << filename << '\n';

                    /* Reject incomplete input */
                    return false;
                /* End this block and return to the enclosing scope */
                }

                /* Store zero for an empty cell */
                if (ch == '_') board[row][col] = 0;

                /* Accept digits 1-9 */
                else if (ch >= '1' && ch <= '9') {

                    /* Convert character to int */
                    board[row][col] = ch - '0';

                /* Reject unexpected characters */
                } else {
                    /* Print the file name when an unexpected character is read */
                    std::cerr << "Invalid character in " << filename << '\n';

                    /* Report invalid input */
                    return false;
                /* End this block and return to the enclosing scope */
                }
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* Check whether more non-whitespace data remains */
        char extra;

        /* Reject files with more than 81 cells */
        if (input >> extra) {
            /* Print the file name when extra puzzle data is found */
            std::cerr << "Extra data in " << filename << '\n';

            /* Report invalid input length */
            return false;
        /* End this block and return to the enclosing scope */
        }

        /* Check original clues */
        for (int row = 0; row < 9; ++row) {

            /* Check each given digit */
            for (int col = 0; col < 9; ++col) {

                /* Remember the given value */
                int digit = board[row][col];

                /* Blanks cannot conflict */
                if (digit == 0) continue;

                /* Temporarily remove the clue */
                board[row][col] = 0;

                /* Test against others */
                bool valid = canPlace(row, col, digit);

                /* Restore the original clue */
                board[row][col] = digit;

                /* Detect conflicting given digits */
                if (!valid) {
                    /* Print the file name when the starting digits violate Sudoku rules */
                    std::cerr << "Conflicting clues in " << filename << '\n';

                    /* Reject contradictory puzzle input */
                    return false;
                /* End this block and return to the enclosing scope */
                }
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* Input is well-formed and internally consistent */
        return true;
    /* End this block and return to the enclosing scope */
    }

    /* Print the puzzle, enumerate solutions, and report unsolvable puzzles */
    void solve(const std::string& filename) {

        /* Print the puzzle file name */
        std::cout << "Puzzle file: " << filename << '\n';

        /* Print the heading for the original puzzle */
        std::cout << "Original puzzle:\n";

        /* Print original puzzle before any modifications */
        printBoard();

        /* Add space before the solutions */
        std::cout << '\n';

        /* Enumerate all solutions using recursive backtracking */
        search();

        /* Handle puzzles with no valid completion */
        if (solutionCount == 0) {

            /* Required error message */
            std::cout << "No solution found\n";
        /* End this block and return to the enclosing scope */
        }
        /* Print the total number of solutions and separate the next puzzle */
        std::cout << "Total solutions: " << solutionCount << "\n\n";
    /* End this block and return to the enclosing scope */
    }
/* End the class definition */
};

/* Process all five puzzle files with separate solver objects */
int main() {

    /* Track whether any input file failed */
    bool allLoaded = true;

    /* Visit each file */
    for (int number = 1; number <= 5; ++number) {
        /* Build the puzzle file name using the current file number */
        const std::string filename = "puzzle" + std::to_string(number) + ".txt";

        /* Construct an independent solver object */
        SudokuSolver solver;

        /* Load and validate the puzzle */
        if (solver.load(filename)) {

            /* Print all valid solutions */
            solver.solve(filename);

        /* Record an invalid or unavailable file */
        } else {

            /* Preserve error status for exit code */
            allLoaded = false;
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

    /* Signal success only if all files loaded */
    return allLoaded ? 0 : 1;
/* End this block and return to the enclosing scope */
}
