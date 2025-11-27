#include <windows.h>  // For UTF-8 console

#include <algorithm>   // for std::shuffle
#include <cctype>      // for std::tolower
#include <filesystem>  // C++17 feature
#include <fstream>     // for file I/O
#include <iostream>    // for std::cout, std::endl
#include <numeric>     // for std::iota
#include <random>      // for std::mt19937 and std::random_device
#include <utility>     // for std::pair

#include "Board.h"

#if defined(_WIN32) || defined(_WIN64)  // Windows specific
#include <conio.h>                      // For _getch()
char getKeyPress() { return _getch(); }
#else  // Unix-like systems
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

char getKeyPress() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) return 0;
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) return 0;
    if (read(0, &buf, 1) < 0) buf = 0;
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSANOW, &old);
    return buf;
}
#endif

// Constructor for Board
Board::Board(int r, int c, int mines)             // Initialize board with given rows, columns, and mines
    : rows(r),
      cols(c),
      totalMines(mines),                         // Initialize member variables
      mine(r, std::vector<bool>(c, false)),      // Initialize mine vector
      revealed(r, std::vector<bool>(c, false)),  // Initialize revealed vector
      neighbors(r, std::vector<int>(c, 0)),       // Initialize neighbors vector
      flagged(r, std::vector<bool>(c, false))     // Initialize flagged vector
{
    std::vector<int> idx(rows * cols);  // Create a vector of indices
    std::iota(idx.begin(), idx.end(), 0);  // Fill with 0, 1, 2, ..., rows*cols-1

    std::random_device rd;  // Seed the random number generator
    std::mt19937 gen(rd());
    std::shuffle(idx.begin(), idx.end(), gen);  // Shuffle the indices

    for (int i = 0; i < totalMines; i++)
        mine[idx[i] / cols][idx[i] % cols] =
            true;  // Place mines at shuffled indices

    countNeighbors();
}

// Check if coordinates are inside the board
bool Board::inBounds(int r, int c) const {
    return r >= 0 && r < rows && c >= 0 &&
           c < cols;  // Return true if (r, c) is within bounds
}

// Count mines around each cell
void Board::countNeighbors() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (mine[r][c]) continue;
            int cnt = 0;
            for (int dr = -1; dr <= 1; dr++)
                for (int dc = -1; dc <= 1; dc++) {
                    int rr = r + dr, cc = c + dc;
                    if (inBounds(rr, cc) && mine[rr][cc]) cnt++;
                }
            neighbors[r][c] = cnt;
        }
    }
}

// Reveal all mines
void Board::revealAllMines() {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (mine[r][c]) revealed[r][c] = true;
}

// Reveal a cell
void Board::revealCell(int r, int c) {
    if (!inBounds(r, c) || revealed[r][c] || flagged[r][c]) return;
    revealed[r][c] = true;

    if (mine[r][c]) {
        lost = true;
        revealAllMines();
    }
}

// Toggle flag on a cell
void Board::toggleFlag(int r, int c) {
    if (!inBounds(r, c) || revealed[r][c]) return;
    flagged[r][c] = !flagged[r][c];
}

// Check if all non-mine cells are revealed
bool Board::isWin() const {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (!mine[r][c] && !revealed[r][c]) return false;
    return true;
}

// Check if player lost
bool Board::isLost() const { return lost; }

// Print board
void Board::print() const {
    std::cout << "     ";
    for (int c = 0; c < cols; c++)
        if (c + 1 < 9)
            std::cout << c + 1 << "   ";
        else
            std::cout << c + 1 << "  ";
    std::cout << "\n";
    std::cout << "   ";
    for (int c = 0; c < cols; c++) std::cout << "+---";
    std::cout << "+\n";

    for (int r = 0; r < rows; r++) {
        std::cout << " " << char('a' + r) << " ";
        for (int c = 0; c < cols; c++) {
            if (revealed[r][c]) {
                if (mine[r][c])
                    std::cout << "| X ";
                else
                    std::cout << "| " << neighbors[r][c] << " ";
            } else {
                if (flagged[r][c])
                    std::cout << "| F ";
                else
                    std::cout << "|   ";
            }
        }
        std::cout << "|\n";
        std::string t1 = "+";
        if (r < rows - 1) {
            std::cout << "   |";
            for (int c = 0; c < cols; c++) {
                if (c == (cols - 1))
                    t1 = "|";
                else
                    t1 = "+";
                std::cout << "---" + t1;
            }
            std::cout << "\n";
        }
    }

    std::cout << "   ";
    for (int c = 0; c < cols; c++) std::cout << "+---";
    std::cout << "+\n";
}

// Save to file
std::pair<std::string, bool> Board::saveToFile(
    const std::string& filename) const {
    std::string outFilename = filename;
    while (std::filesystem::exists(outFilename + ".txt")) {
        std::cout << "Fil \"" << outFilename << ".txt\" finns redan.\n"
                  << "Skriva över filen? (y/n): ";
        while (true) {
            char choice = getKeyPress();
            std::cout << choice << "\n";
            if (choice == 'y' || choice == 'Y') {
                goto proceed;
            } else if (choice == 'n' || choice == 'N') {
                std::cout << "Ange nytt filnamn (utan filändelse): ";
                std::string newName;
                std::cin >> newName;
                if (!newName.empty()) {
                    outFilename = newName;
                    break;  // break inner loop to recheck new filename
                }
            } else {
                std::cout << "Invalid input. Please enter 'y' or 'n'.\n";
            }
        }
    }
proceed:
    std::ofstream out(outFilename + ".txt");
    if (!out) return {outFilename, false};

    out << rows << " " << cols << " " << totalMines << "\n";

    for (auto& row : mine) {
        for (bool b : row) out << (b ? 1 : 0) << " ";
        out << "\n";
    }

    for (auto& row : revealed) {
        for (bool b : row) out << (b ? 1 : 0) << " ";
        out << "\n";
    }

    for (auto& row : flagged) {
        for (bool b : row) out << (b ? 1 : 0) << " ";
        out << "\n";
    }

    return {outFilename, true};
}

// Load from file
bool Board::loadFromFile(const std::string& filename) {
    std::ifstream in(filename + ".txt");
    if (!in) return false;

    in >> rows >> cols >> totalMines; // Read dimensions and mine count
    if (!in) return false;

    mine.assign(rows, std::vector<bool>(cols, false));  // Resize and clear
    revealed.assign(rows, std::vector<bool>(cols, false));  // Resize and clear
    neighbors.assign(rows, std::vector<int>(cols, 0));  // Resize and clear

    int val;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            in >> val;    // Read mine data
            mine[r][c] = (val == 1);
        }

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            in >> val;    // Read revealed data
            revealed[r][c] = (val == 1);
        }

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            in >> val;  // Read flagged data
            flagged[r][c] = (val == 1);
    }

    countNeighbors();
    lost = false;
    return true;  //
}

// Parse coordinates from string (e.g., "b2") into row and column
bool parseCoord(const std::string& s, int& r, int& c) {
    if (s.size() < 2) return false;

    unsigned char uc = static_cast<unsigned char>(s[0]);
    if (!std::isalpha(uc)) return false;

    char ch = static_cast<char>(std::tolower(uc));
    if (ch < 'a' || ch > 'z') return false;
    r = ch - 'a';

    try {
        int val = std::stoi(s.substr(1));
        if (val < 1) return false;
        c = val - 1;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
    return true;
}
