#ifndef DEFS_INCLUDED
#define DEFS_INCLUDED

#include <stdint.h>
#include <cassert>
#include <string>

namespace ChessEngine {

using Bitboard = uint64_t;

const std::string startPosFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr int MAX_MOVES = 256;

// A move consist of the following four fields:
//    
// bit   0-5:    source Square
// bit  6-11:    target Square
// bit 12-13:    MoveType
// bit 14-15:    promotion piece (PieceType - KNIGHT)
enum Move : uint16_t
{
  MOVE_NONE,
  MOVE_NULL = 65
};

enum MoveType
{
  NORMAL,
  PROMOTION,
  EN_PASSANT,
  CASTLING 
};

enum Direction : int
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

enum PieceType
{
    NO_TYPE,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    NUM_PIECE_TYPES = 8
};


// The third LSB indicates a sliding piece.
// The fourth LSB indicates the color of the piece.
enum Piece : uint8_t
{
    EMPTY,
    
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,

    BLACK_PAWN = 9,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,

    NUM_PIECES = NUM_PIECE_TYPES * NUM_COLORS
};

enum CastlingRight : uint8_t
{
    NO_CASTLING,
    WHITE_SHORT = 1,
    WHITE_LONG  = 2,
    BLACK_SHORT = 4,
    BLACK_LONG  = 8,

    KING_SIDE      = WHITE_SHORT    | BLACK_SHORT,
    QUEEN_SIDE     = WHITE_LONG     | BLACK_LONG,
    BLACK_CASTLING = BLACK_SHORT    | BLACK_LONG,
    WHITE_CASTLING = WHITE_SHORT    | WHITE_LONG,
    ANY_CASTLING   = WHITE_CASTLING | BLACK_CASTLING,
};

// First three LSB indicates the file, rest of the bits the rank.
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
    NUM_SQUARES, NO_SQUARE
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


// Enable increment and decrement operators on enums
#define ENABLE_INCR_OPERATORS_ON(T)                                                                     \
inline T operator++(T& d, int) { T previous = d; d = static_cast<T>(int(d) + 1); return previous; }     \
inline T operator--(T& d, int) { T previous = d; d = static_cast<T>(int(d) - 1); return previous; }

ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)

#undef ENABLE_INCR_OPERATORS_ON

constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }


constexpr Square createSquare(File file, Rank rank) 
{
    return Square((rank << 3) + file);
}

constexpr File getFile(Square square)
{
    return File(square & 0b111);
}

constexpr Rank getRank(Square square) 
{
    return Rank(square >> 3);
}

constexpr Rank relativeRank(Rank rank, Color color)
{
    return Rank(rank ^ (color * 7));
}

constexpr Piece getPiece(PieceType pt, Color color) 
{
    return Piece(pt | (color << 3));
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

inline Color operator~(Color color) {
    return Color(color ^ BLACK);
}

constexpr Direction getPawnDir(Color color)
{
    return color == WHITE ? NORTH : SOUTH;
}

constexpr Square getFromSquare(Move move)
{
    return Square(move & 0x3F);
}

constexpr Square getToSquare(Move move)
{
    return Square((move >> 6) & 0x3F);
}

constexpr MoveType getMoveType(Move move)
{
    return MoveType((move >> 12) & 3);
}

constexpr PieceType getPromotionType(Move move)
{
    // As the promotion flag is only two bits the type is offset by 2 (a KNIGHT)
    return PieceType(((move >> 14) & 3) + KNIGHT);
}

constexpr Move makeQuietMove(Square from, Square to)
{
    return Move((to << 6) + from);
}

constexpr Move makeMove(Square from, Square to, MoveType mt, PieceType promotionPt = KNIGHT)
{
    return Move(((promotionPt - KNIGHT) << 14) + (mt << 12) + (to << 6) + from);
}

inline std::string algebraicNotation(Square square)
{
    return { char('a' + getFile(square)), char('1' + getRank(square)) };
}

} // namespace ChessEngine

#endif // DEFS_INCLUDED