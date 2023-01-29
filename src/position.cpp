#include <iostream>
#include <cstring>
#include <sstream>
#include <string_view>

#include "position.h"

namespace ChessEngine {

namespace {

constexpr std::string_view PieceToChar(" PNBRQK  pnbrqk");

} // namespace

void Position::init()
{

}

Position& Position::setPosFromFEN(const std::string& fen)
{
    std::memset(this, 0, sizeof(Position));

    std::istringstream ss(fen);
    ss >> std::noskipws;

    uint8_t token;
    int square = A8; // FEN reads from left to right starting from the top rank

    while ((ss >> token) && !isspace(token))
    {
        if (isdigit(token))
            square += (token - '0');

        else if (token == '/')
            square += 2*SOUTH;
        
        //else if ()
        
    }
    
}

void Position::print()
{
    //std::cout << "        " << bitboard << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+---+\n";

    for (int rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (int file = FILE_A; file < NUM_FILES; file++)
        {
           std::cout << "| " << PieceToChar[board[getSquare(File(file), Rank(rank))]] << " ";
        }

        std::cout << "|\n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n" << std::endl;
}

} // namespace ChessEngine