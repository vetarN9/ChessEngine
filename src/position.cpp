#include <iostream>
#include <cstring>
#include <sstream>
#include <string_view>
#include <map>

#include "position.h"

namespace ChessEngine {

void Position::init()
{

}


/*
    A FEN record defines a particular game position, all in one text line and using only the ASCII character set. 
    A text file with only FEN data records should use the filename extension .fen.

    A FEN record contains six fields, each separated by a space. The fields are as follows:

    1. Piece placement data: Each rank is described, starting with rank 8 and ending with rank 1, with a "/" 
       between each one; within each rank, the contents of the squares are described in order from the a-file 
       to the h-file. Each piece is identified by a single letter taken from the standard English names in algebraic
       notation (pawn = "P", knight = "N", bishop = "B", rook = "R", queen = "Q" and king = "K"). White pieces are
       designated using uppercase letters ("PNBRQK"), while black pieces use lowercase letters ("pnbrqk"). 
       A set of one or more consecutive empty squares within a rank is denoted by a digit from "1" to "8",
       corresponding to the number of squares.
    
    2. Active color: "w" means that White is to move; "b" means that Black is to move.
    
    3. Castling availability: If neither side has the ability to castle, this field uses the character "-".
       Otherwise, this field contains one or more letters: "K" if White can castle kingside, "Q" if White can 
       castle queenside, "k" if Black can castle kingside, and "q" if Black can castle queenside. A situation that 
       temporarily prevents castling does not prevent the use of this notation.
    
    4. En passant target square: This is a square over which a pawn has just passed while moving two squares; 
       it is given in algebraic notation. If there is no en passant target square, this field uses the character "-". 
       This is recorded regardless of whether there is a pawn in position to capture en passant.[6] An updated 
       version of the spec has since made it so the target square is only recorded if a legal en passant move is 
       possible but the old version of the standard is the one most commonly used.
    
    5. Halfmove clock: The number of halfmoves since the last capture or pawn advance, used for the fifty-move rule.
    
    6. Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.

    Example:
        FEN starting posiion: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
*/

Position& Position::setPosFromFEN(const std::string& fen)
{
    std::memset(this, 0, sizeof(Position));

    std::istringstream ss(fen);
    ss >> std::noskipws;

    uint8_t token;
    int square = A8; // FEN reads from left to right starting from the top rank

    constexpr std::string_view PieceToChar(" PNBRQK  pnbrqk");

    // 1. Piece placement data
    while ((ss >> token) && !isspace(token))
    {
        if (isdigit(token))
            square += (token - '0'); 

        else if (token == '/')
            square += 2*SOUTH;
        
        else
        {
            placePiece(Piece(PieceToChar.find(token)), Square(square));
            square++;
        }
        
    }

    // 2. Active color
    ss >> token;
    sideToMove = (token == 'w') ? WHITE : BLACK;
    ss >> token;
    
    // 3. Castling availability
    while ((ss >> token) && !isspace(token))
    {
        
    }

    // 4. En passant target square


    // 5. Halfmove clock


    // 6. Fullmove number


    return *this;
}

void Position::print()
{
    std::map<Piece, std::string> pieceToChar = {
        {EMPTY,          " "},
        {WHITE_PAWN,     "♟"},
        {WHITE_KNIGHT,   "♞"},
        {WHITE_BISHOP,   "♝"},
        {WHITE_ROOK,     "♜"},
        {WHITE_QUEEN,    "♛"},
        {WHITE_KING,     "♚"},
        {BLACK_PAWN,     "♙"},
        {BLACK_KNIGHT,   "♘"},
        {BLACK_BISHOP,   "♗"},
        {BLACK_ROOK,     "♖"},
        {BLACK_QUEEN,    "♕"},
        {BLACK_KING,     "♔"}
    };

    std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;

    for (int rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (int file = FILE_A; file < NUM_FILES; file++)
        {
            std::cout << "| " << pieceToChar[pieceBoard[getSquare(File(file), Rank(rank))]] << " ";
        }

        std::cout << "|\n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n" << std::endl;
}

} // namespace ChessEngine