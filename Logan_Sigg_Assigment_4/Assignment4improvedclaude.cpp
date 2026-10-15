/*
 * Program: EECS 348 Assignment 4
 *
 * Description:
 * This C++ program reads and solves five Sudoku puzzles
 * The program uses recursive depth-first search with backtracking
 * to find every valid solution and prints each solution as it is found
 * Each puzzle is printed before its solutions
 * Puzzles without a valid solution display "No solution found"
 *
 * Inputs:
 * puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, and puzzle5.txt
 * Each file contains 81 cells with digits 1 through 9 and underscores or zeros for blanks
 * Spaces and newlines between cells are optional
 *
 * Outputs:
 * The file name, original puzzle, every valid solution, and solution count
 * A message when no solution is found and solve times sent to standard error
 *
 * Collaborators:
 * None
 *
 * Other Sources:
 * Claude was used to generate the initial and improved C++ implementations
 * The assignment instructions provided the program requirements
 *
 * Author: Logan Sigg
 * Creation Date: 10/11/2026
 * Revision Date: 10/14/2026
 *
 */

/* Console output */
#include <iostream>
/* Reading puzzle files */
#include <fstream>
/* Std::string for file names */
#include <string>
/* List of blank cells */
#include <vector>
/* Fixed-size grid storage */
#include <array>
/* Portable bit counting */
#include <bitset>
/* Timing each solve */
#include <chrono>
/* Std::function callback for found solutions */
#include <functional>

/* Grid is SIZE x SIZE */
const int SIZE = 9;
/* Each block is BOX x BOX */
const int BOX  = 3;
/* Bits 1..9 set (bit 0 unused) */
const int ALL_DIGITS = 0x3FE;

/* Class Board: a 9x9 grid plus bitmasks of digits already used in every row, */
/* Column and block, so a legality check is a few bit operations */
class Board {
/* These members can be accessed outside the class */
public:
    /* Start empty */
    Board() { clear(); }

    /* Reset grid and masks */
    void clear() {
        /* Each row/col/box index */
        for (int i = 0; i < SIZE; ++i) {
            /* No digits used */
            rowMask[i] = colMask[i] = boxMask[i] = 0;
            /* 0 means blank */
            for (int c = 0; c < SIZE; ++c) cells[i][c] = 0;
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

    /* Load a puzzle. Reads character by character, so "5 3 _" and "53_" both work */
    /* Returns false if the file is missing/malformed or the clues already conflict */
    bool loadFromFile(const std::string& fileName) {
        /* Open the file */
        std::ifstream in(fileName);
        /* Cannot open */
        if (!in) return false;
        /* Wipe old state */
        clear();
        /* Cells read so far */
        int count = 0;
        /* Current character */
        char ch;
        /* Read every character */
        while (in.get(ch)) {
            /* Skip spacing */
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') continue;
            /* Extra data after 81 cells */
            if (count >= SIZE * SIZE) return false;
            /* Cell position */
            int r = count / SIZE, c = count % SIZE;
            /* One more cell consumed */
            ++count;
            /* Blank stays 0 */
            if (ch == '_' || ch == '0') continue;
            /* Illegal character */
            if (ch < '1' || ch > '9') return false;
            /* Char to digit */
            int d = ch - '0';
            /* Clue breaks a rule */
            if (!canPlace(r, c, d)) return false;
            /* Record clue */
            place(r, c, d);
        /* End this block and return to the enclosing scope */
        }
        /* Must have exactly 81 cells */
        return count == SIZE * SIZE;
    /* End this block and return to the enclosing scope */
    }

    /* Read a cell */
    int get(int r, int c) const { return cells[r][c]; }

    /* Is d legal at (r,c)? */
    bool canPlace(int r, int c, int d) const {
        /* Check its candidate bit */
        return (candidates(r, c) >> d) & 1;
    /* End this block and return to the enclosing scope */
    }

    /* Put d into the cell */
    void place(int r, int c, int d) {
        /* Bit for digit d */
        int bit = 1 << d;
        /* Store in grid */
        cells[r][c] = d;
        /* Used in row */
        rowMask[r] |= bit;
        /* Used in column */
        colMask[c] |= bit;
        /* Used in block */
        boxMask[boxOf(r, c)] |= bit;
    /* End this block and return to the enclosing scope */
    }

    /* Undo a placement */
    void remove(int r, int c) {
        /* Digit being removed */
        int bit = 1 << cells[r][c];
        /* Free in row */
        rowMask[r] &= ~bit;
        /* Free in column */
        colMask[c] &= ~bit;
        /* Free in block */
        boxMask[boxOf(r, c)] &= ~bit;
        /* Blank again */
        cells[r][c] = 0;
    /* End this block and return to the enclosing scope */
    }

    /* Bitmask of allowed digits */
    int candidates(int r, int c) const {
        return ~(rowMask[r] | colMask[c] | boxMask[boxOf(r, c)]) & ALL_DIGITS;
    /* End this block and return to the enclosing scope */
    }

    /* Print the grid */
    void print(std::ostream& out) const {
        /* Each row */
        for (int r = 0; r < SIZE; ++r) {
            /* Each column */
            for (int c = 0; c < SIZE; ++c)
                out << (cells[r][c] == 0 ? '_' : char('0' + cells[r][c])) << ' ';
            /* End of row */
            out << '\n';
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }

/* These members are used only within the class */
private:
    /* Block index */
    static int boxOf(int r, int c) { return (r / BOX) * BOX + c / BOX; }

    /* The grid (0 = blank) */
    std::array<std::array<int, SIZE>, SIZE> cells;
    /* Used-digit bitmasks */
    std::array<int, SIZE> rowMask, colMask, boxMask;
/* End the class definition */
};

/* Class SudokuSolver: recursive backtracking. Solutions are handed to a callback */
/* As they are found, so none have to be stored */
class SudokuSolver {
/* These members can be accessed outside the class */
public:
    /* Called once per solution */
    using Callback = std::function<void(const Board&)>;

    /* Search all solutions of the puzzle; returns how many were found */
    int solveAll(Board puzzle, const Callback& onSolution) {
        /* Rebuild the blank list */
        blanks.clear();
        /* Scan the puzzle once */
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (puzzle.get(r, c) == 0) blanks.push_back({r, c});
        /* Remember the callback */
        callback = &onSolution;
        /* Reset the counter */
        found = 0;
        /* Start recursion */
        search(puzzle);
        /* Number of solutions */
        return found;
    /* End this block and return to the enclosing scope */
    }

/* These members are used only within the class */
private:
    /* Coordinates of originally blank cells */
    std::vector<std::pair<int, int>> blanks;
    /* Where solutions are sent */
    const Callback* callback = nullptr;
    /* Solutions so far */
    int found = 0;

    /* Pick the blank cell with the fewest candidates, try each, recurse, undo */
    void search(Board& b) {
        /* Best blank so far */
        int bestIdx = -1, bestCands = 0, bestCount = SIZE + 1;
        /* Only look at blank cells */
        for (std::size_t i = 0; i < blanks.size(); ++i) {
            /* Cell coordinates */
            int r = blanks[i].first, c = blanks[i].second;
            /* Already filled on this path */
            if (b.get(r, c) != 0) continue;
            /* Allowed digits */
            int cands = b.candidates(r, c);
            /* How many */
            int n = (int)std::bitset<SIZE + 1>(cands).count();
            /* Dead end: backtrack */
            if (n == 0) return;
            /* Most constrained so far */
            if (n < bestCount) {
                bestCount = n; bestIdx = (int)i; bestCands = cands;
                /* Cannot do better; stop scanning */
                if (n == 1) break;
            /* End this block and return to the enclosing scope */
            }
        /* End this block and return to the enclosing scope */
        }
        /* No blanks left => solved */
        if (bestIdx == -1) {
            /* Count it */
            ++found;
            /* Report it right away */
            (*callback)(b);
            /* Keep looking for more */
            return;
        /* End this block and return to the enclosing scope */
        }
        /* Chosen cell */
        int r = blanks[bestIdx].first, c = blanks[bestIdx].second;
        /* Try each digit */
        for (int d = 1; d <= SIZE; ++d) {
            /* Not a candidate */
            if (!((bestCands >> d) & 1)) continue;
            /* Tentatively place */
            b.place(r, c, d);
            /* Recurse */
            search(b);
            /* Backtrack */
            b.remove(r, c);
        /* End this block and return to the enclosing scope */
        }
    /* End this block and return to the enclosing scope */
    }
/* End the class definition */
};

/* Main: solve puzzle1.txt ... puzzle5.txt and report results */
int main() {
    /* Reused for every file */
    SudokuSolver solver;
    /* Five puzzle files */
    for (int i = 1; i <= 5; ++i) {
        std::string name = "puzzle" + std::to_string(i) + ".txt";
        std::cout << "==============================\nFile: " << name << "\n\n";
        /* Board to load into */
        Board puzzle;
        /* Read and validate */
        if (!puzzle.loadFromFile(name)) {
            std::cout << "Could not read a valid puzzle from " << name << "\n\n";
            /* Next file */
            continue;
        /* End this block and return to the enclosing scope */
        }
        /* Show the original */
        std::cout << "Puzzle:\n";
        puzzle.print(std::cout);
        /* Solutions printed so far */
        int shown = 0;
        /* Start timer */
        auto t0 = std::chrono::steady_clock::now();
        /* Print as found */
        int total = solver.solveAll(puzzle, [&](const Board& sol) {
            std::cout << "\nSolution " << ++shown << ":\n";
            sol.print(std::cout);
        });
        /* Stop timer */
        auto t1 = std::chrono::steady_clock::now();
        if (total == 0) std::cout << "\nNo solution found\n\n";
        else std::cout << "\n" << total << " solution(s) found\n\n";
        std::cerr << name << " solve time: "
                  << std::chrono::duration<double, std::milli>(t1 - t0).count() << " ms\n";
    /* End this block and return to the enclosing scope */
    }
    /* Done */
    return 0;
/* End this block and return to the enclosing scope */
}
