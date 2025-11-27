#include <windows.h>  // For UTF-8 console

#include <filesystem>  // for directory iteration
#include <iostream>    // for std::cout, std::endl
#include <memory>      // for std::unique_ptr, std::make_unique

#include "Board.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::unique_ptr<Board> board =
        std::make_unique<Board>(6, 6, 6);  // Default board

    std::cout << "\n=========================================\n";
    std::cout << "   Textbaserat Minröj - skriv t.ex. b2 \n";
    std::cout << "   Kommandon: save / load \n";
    std::cout << "=========================================\n\n";

    std::string lastInput;

    while (true) {
        board->print();

        if (board->isLost()) {
            std::cout << "Pang!! Game Over. Ruta " << lastInput
                      << " innehöll en bomb\n\n";
            break;
        }
        if (board->isWin()) {
            std::cout << "Grattis! Du vann!\n";
            break;
        }

        std::cout << "<> Vilken ruta vill du undersöka?\n";
        std::cout << "<> Skriv 'save' för att spara spelet.\n";
        std::cout << "<> Skriv 'load' för att ladda ett annat spel.\n> ";

        std::string cmd;
        std::cin >> cmd;
        lastInput = cmd;
        std::cout << "========================================================="
                     "=========\n";

        if (cmd == "save") {  // Save game
            std::string fname;
            std::cout << "Ange filnamn (utan filändelse): ";
            std::cin >> fname;
            auto result = board->saveToFile(fname);
            if (result.second)
                std::cout << "Spelet sparat i " << result.first << ".txt"
                          << "\n";
            else
                std::cout << "( Kunde inte spara spelet! )\n";
            continue;
        }

        if (cmd == "load") {  // Load game
            namespace fs = std::filesystem;
            std::string path = ".";  // current directory

            std::cout << "Filename\t\tLast Modified\n";
            std::cout << "---------------------------------------\n";

            for (const auto& entry : fs::directory_iterator(path)) {
                if (entry.is_regular_file() &&
                    entry.path().extension() == ".txt") {
                    // Get last write time
                    auto ftime = fs::last_write_time(entry);
                    auto sctp = std::chrono::time_point_cast<
                        std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() +
                        std::chrono::system_clock::now());
                    std::time_t cftime =
                        std::chrono::system_clock::to_time_t(sctp);

                    // Print filename and last modified date
                    std::cout << std::left << std::setw(20)
                              << entry.path().filename().string()
                              << std::put_time(std::localtime(&cftime),
                                               "%Y-%m-%d %H:%M:%S")
                              << "\n";
                }
            }

            std::string fname;  // Get filename to load
            std::cout << "Ange filnamn (utan filändelse): ";
            std::cin >> fname;
            if (board->loadFromFile(fname))
                std::cout << "Spelet laddat från " << fname << ".txt" << "\n";
            else
                std::cout << "( Filen inte existerar eller inte kan läsas! )\n";
            continue;
        }

        int r, c;  // Parse coordinates
        if (!parseCoord(cmd, r, c)) {   
            std::cout << "Ogiltig koordinat, försök igen!\n";
            continue;
        }

        if (!board->inBounds(r, c)) {  // Check bounds
            std::cout << "Utanför gränserna, försök igen!\n";
            continue;
        }

        board->revealCell(r, c);
    }

    return 0;
}
