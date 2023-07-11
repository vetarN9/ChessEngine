#ifndef DEFS_INCLUDED
#define DEFS_INCLUDED

#include <stdint.h>
#include <cassert>

namespace ChessEngine {

typedef uint64_t Bitboard;

const std::string startPosFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum Direction : int8_t
{
    NORTH =  8,
    SOUTH = -8,
    EAST  =  1,
    WEST  = -1,

    NORTH_EAST = NORTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    SOUTH_EAST = SOUTH + EAST,
    NORTH_WEST = NORTH + WEST
};

enum Color
{
    WHITE, BLACK,
    NUM_COLORS
};

enum PieceType {
    NO_TYPE,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    NUM_PIECE_TYPES = 8
};

/**
 * @brief Enum for all the pieces. The third LSB indicates a sliding piece.
 * The fourth LSB indicates the color of the piece.
 */
enum Piece : uint8_t
{
    EMPTY,
    WHITE_PAWN,     WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN = 9, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

enum Square
{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NUM_SQUARES
};

enum File
{
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
    NUM_FILES
};

enum Rank
{
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
    NUM_RANKS
};

constexpr Square getSquare(File file, Rank rank) 
{
    return Square((rank * 8) + file);
}

constexpr Piece getPiece(PieceType pieceType, Color color) 
{
    return Piece(pieceType | (color << 3));
}

constexpr PieceType getType(Piece piece) 
{
    return PieceType(piece & 0b111);
}

constexpr Color getColor(Piece piece) 
{
    assert(piece != EMPTY);
    return Color(piece >> 3);
}

} // namespace ChessEngine

#endif // DEFS_INCLUDED