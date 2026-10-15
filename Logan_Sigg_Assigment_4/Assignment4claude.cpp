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
 * Claude was used to generate the initial C++ implementation
 * The assignment instructions provided the program requirements
 *
 * Author: Logan Sigg
 * Creation Date: 10/11/2026
 * Revision Date: 10/14/2026
 *
 */

/*
 * These libraries provide terminal I/O, file handling, strings,
 * storage for solutions with vector, and fixed-size arrays for the board
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

/*
 * The Board class stores a 9-by-9 grid and the digits used in each row,
 * column, and 3-by-3 box Bitmasks are used to check available digits
 */
class Board {
/* These members can be accessed outside the class */
public:

    /* Start with an empty board */
    Board() { clear(); }

    /* Reset grid and masks */
    void clear() {

        /* Go through every row */
        for (int r = 0; r < 9; ++r) {

            /* No digits used yet */
            rowMask[r] = colMask[r] = boxMask[r] = 0;

            /* 0 means "blank" */
            for (int c = 0; c < 9; ++c) cells[r][c] = 0;
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

    /*
     * Load the board from a puzzle file
     * Return false if the file cannot be read or the starting clues are invalid
     */
    bool loadFromFile(const std::string& fileName) {

        /* Open the puzzle file */
        std::ifstream in(fileName);

        /* Return false if the puzzle file could not be opened */
        if (!in) return false;

        /* Clear the previous puzzle and its digit masks */
        clear();

        /* Read each of the nine rows */
        for (int r = 0; r < 9; ++r) {

            /* Read each of the nine cells in the row */
            for (int c = 0; c < 9; ++c) {

                /* One whitespace-separated token */
                std::string tok;

                /* File ended too early */
                if (!(in >> tok)) return false;

                /* Blank cell stays 0 */
                if (tok == "_") continue;

                /* Reject tokens other than underscores or digits from 1 to 9 */
                if (tok.size() != 1 || tok[0] < '1' || tok[0] > '9') return false;

                /* Convert char to digit */
                int d = tok[0] - '0';

                /* Clue conflicts with another clue */
                if (!canPlace(r, c, d)) return false;

                /* Record the clue */
                place(r, c, d);
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* Return true after loading a valid puzzle */
        return true;
    /* End this block and return to the enclosing scope */
    }

    /* Read one cell */
    int  get(int r, int c) const { return cells[r][c]; }

    /* Check the row, column, and box masks to see whether the digit can be placed */
    bool canPlace(int r, int c, int d) const {

        /* Bit for this digit */
        int bit = 1 << d;
        /* Return true if the digit is unused in the row, column, and box */
        return !((rowMask[r] | colMask[c] | boxMask[boxOf(r, c)]) & bit);
    /* End this block and return to the enclosing scope */
    }

    /* Put digit d into the cell */
    void place(int r, int c, int d) {

        /* Bit for this digit */
        int bit = 1 << d;

        /* Store it in the grid */
        cells[r][c] = d;

        /* Mark used in row */
        rowMask[r] |= bit;

        /* Mark used in column */
        colMask[c] |= bit;

        /* Mark used in block */
        boxMask[boxOf(r, c)] |= bit;
    /* End this block and return to the enclosing scope */
    }

    /* Undo a placement (backtrack) */
    void remove(int r, int c) {

        /* Bit of the digit being removed */
        int bit = 1 << cells[r][c];

        /* Free it in the row */
        rowMask[r] &= ~bit;

        /* Free it in the column */
        colMask[c] &= ~bit;

        /* Free it in the block */
        boxMask[boxOf(r, c)] &= ~bit;

        /* Blank the cell again */
        cells[r][c] = 0;
    /* End this block and return to the enclosing scope */
    }

    /* Bitmask of every digit still allowed at (r,c) (bit d set => d is a candidate) */
    int candidates(int r, int c) const {

        /* Keep only the bits for digits 1 through 9 */
        return ~(rowMask[r] | colMask[c] | boxMask[boxOf(r, c)]) & 0x3FE;
    /* End this block and return to the enclosing scope */
    }

    /* Print the current board */
    void print(std::ostream& out) const {

        /* Each row */
        for (int r = 0; r < 9; ++r) {

            /* Each column */
            for (int c = 0; c < 9; ++c) {

                /* Blank shown as underscore */
                if (cells[r][c] == 0) out << "_ ";

                /* Print the digit for a filled cell */
                else out << cells[r][c] << ' ';
            /* End this block and return to the enclosing scope */
            }

            /* End of row */
            out << '\n';
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

/* These members are used only within the class */
private:

    /* Return the box index from 0 through 8 */
    static int boxOf(int r, int c) { return (r / 3) * 3 + c / 3; }

    /* The grid (0 = blank) */
    std::array<std::array<int, 9>, 9> cells;

    /* Used-digit bitmasks */
    std::array<int, 9> rowMask, colMask, boxMask;
/* End the class definition */
};

/* Class SudokuSolver: recursive backtracking that collects all solutions */
class SudokuSolver {
/* These members can be accessed outside the class */
public:
    /* Solve a copy of the puzzle and return every solution found */
    std::vector<Board> solveAll(const Board& puzzle) {

        /* Forget earlier results */
        solutions.clear();

        /* Work on a copy, keep original */
        Board work = puzzle;

        /* Start the recursion */
        search(work);

        /* Return every solution found */
        return solutions;
    /* End this block and return to the enclosing scope */
    }

/* These members are used only within the class */
private:

    /* Every completed grid found */
    std::vector<Board> solutions;

    /*
     * Select the empty cell with the fewest available digits
     * Try each digit recursively and undo the placement to find other solutions
     */
    void search(Board& b) {

        /* Best blank so far */
        int bestR = -1, bestC = -1, bestCands = 0, bestCount = 10;

        /* Scan every cell */
        for (int r = 0; r < 9; ++r) {
            /* Check each column in the current row */
            for (int c = 0; c < 9; ++c) {

                /* Skip filled cells */
                if (b.get(r, c) != 0) continue;

                /* Allowed digits here */
                int cands = b.candidates(r, c);

                /* Count the available digits */
                int n = __builtin_popcount(cands);

                /* Dead end: backtrack immediately */
                if (n == 0) return;

                /* Select the cell if it has fewer available digits */
                if (n < bestCount) {

                    /* Save the candidate count, row, column, and candidate mask for this cell */
                    bestCount = n; bestR = r; bestC = c; bestCands = cands;
                /* End this block and return to the enclosing scope */
                }
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }

        /* Save a solution when no empty cells remain */
        if (bestR == -1) {

            /* Save this solution */
            solutions.push_back(b);

            /* Return to earlier calls to continue finding solutions */
            return;
        /* End this block and return to the enclosing scope */
        }

        /* Try each digit */
        for (int d = 1; d <= 9; ++d) {

            /* Not a candidate, skip */
            if (!(bestCands & (1 << d))) continue;

            /* Tentatively place it */
            b.place(bestR, bestC, d);

            /* Recurse with one more cell filled */
            search(b);

            /* Remove the digit to try other solutions */
            b.remove(bestR, bestC);
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }
/* End the class definition */
};

/* Main: run the solver on puzzle1.txt ... puzzle5.txt and report results */
int main() {

    /* One solver reused for all files */
    SudokuSolver solver;

    /* Five puzzle files */
    for (int i = 1; i <= 5; ++i) {

        /* Build the file name */
        std::string name = "puzzle" + std::to_string(i) + ".txt";

        /* Visual separator */
        std::cout << "==============================\n";

        /* Print the file name */
        std::cout << "File: " << name << "\n\n";

        /* Board to load into */
        Board puzzle;

        /* Read the puzzle and check its starting clues */
        if (!puzzle.loadFromFile(name)) {
            /* Print the file name when the puzzle cannot be loaded */
            std::cout << "Could not read a valid puzzle from " << name << "\n\n";

            /* Move on to the next file */
            continue;
        /* End this block and return to the enclosing scope */
        }

        /* Label the original puzzle */
        std::cout << "Puzzle:\n";

        /* Print the original puzzle */
        puzzle.print(std::cout);

        /* Find all solutions */
        std::vector<Board> sols = solver.solveAll(puzzle);

        /* Report when no solutions were found */
        if (sols.empty()) {
            /* Print the required message when the puzzle has no solution */
            std::cout << "\nNo solution found\n\n";

        /* Print all solutions found */
        } else {
            /* Print the number of solutions found */
            std::cout << "\n" << sols.size() << " solution(s) found\n";

            /* Print each solution */
            for (std::size_t k = 0; k < sols.size(); ++k) {
                /* Label each solution starting with solution number one */
                std::cout << "\nSolution " << (k + 1) << ":\n";
                /* Print the completed board for this solution */
                sols[k].print(std::cout);
            /* End this block and return to the enclosing scope */
            }
            /* Add a blank line after the solutions */
            std::cout << '\n';
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

    /* Signal successful completion to the operating system */
    return 0;
/* End this block and return to the enclosing scope */
}
