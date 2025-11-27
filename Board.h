#include <string>   // for std::string
#include <utility>  // for std::pair
#include <vector>   // for std::vector

// Class representing the Minesweeper board
class Board {
   public:
    Board(int r, int c, int mines);  // Constructor
    void revealCell(int r, int c);   // Reveal a cell
    void print() const;              // Print the board
    bool isWin() const;              // Check if player won
    bool isLost() const;             // Check if player lost
    bool inBounds(int r,
                  int c) const;  // Check if coordinates are within bounds
    std::pair<std::string, bool> saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

   private:
    int rows, cols, totalMines;
    bool lost = false;

    std::vector<std::vector<bool>> mine;
    std::vector<std::vector<bool>> revealed;
    std::vector<std::vector<int>> neighbors;

    void countNeighbors();
    void revealAllMines();
};

// Parse coordinates from string (e.g., "b2") into row and column
bool parseCoord(const std::string& s, int& r, int& c);
