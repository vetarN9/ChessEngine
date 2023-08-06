#ifndef BITBOARD_INCLUDED
#define BITBOARD_INCLUDED

#include <stdint.h>
#include <cassert>

#include "defs.h"

namespace ChessEngine {

namespace Bitboards {

// Initializes pre-calculated attack tables using magic bitboards
void init();

// Prints the given bitboard to stdout
void print(Bitboard bitboard);

} // namespace Bitboards

constexpr Bitboard Rank1Mask = 0xFF;
constexpr Bitboard Rank8Mask = Rank1Mask << (8 * 7);
constexpr Bitboard FileAMask = 0x0101010101010101;
constexpr Bitboard FileHMask = FileAMask << 7;

// Global pre-calculated tables
extern Bitboard squareMasks[NUM_SQUARES];
extern Bitboard lineMask[NUM_SQUARES][NUM_SQUARES];
extern Bitboard betweenMask[NUM_SQUARES][NUM_SQUARES];
extern Bitboard pawnAttacks[NUM_COLORS][NUM_SQUARES];
extern Bitboard pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

struct Magic 
{
    Bitboard* attacks;
    Bitboard mask;
    Bitboard magic;
    uint32_t shift;

    uint32_t index(Bitboard blockers) const { return ((blockers & mask) * magic) >> shift; }
};

extern Magic bishopMagics[NUM_SQUARES];
extern Magic rookMagics[NUM_SQUARES];

//Operators for modifying a bitboard with a square
inline Bitboard  operator& (Bitboard  bitboard, Square square) { return bitboard &  squareMasks[square]; }
inline Bitboard  operator| (Bitboard  bitboard, Square square) { return bitboard |  squareMasks[square]; }
inline Bitboard  operator^ (Bitboard  bitboard, Square square) { return bitboard ^  squareMasks[square]; }
inline Bitboard& operator|=(Bitboard& bitboard, Square square) { return bitboard |= squareMasks[square]; }
inline Bitboard& operator^=(Bitboard& bitboard, Square square) { return bitboard ^= squareMasks[square]; }

constexpr Bitboard getRankMask(Square square) 
{
    return Rank1Mask << (8 * Rank(square >> 3));
}

constexpr Bitboard getRankMask(Rank rank) 
{
    return Rank1Mask << (8 * rank);
}

constexpr Bitboard getFileMask(Square square) 
{
    return FileAMask << File(square & 0b111);
}

constexpr bool moreThanOne(Bitboard bitboard)
{
    return bitboard & (bitboard - 1);
}

constexpr Bitboard shift(Bitboard bitboard, Direction dir)
{
    // Avoid shifting a negative amount
    switch (dir)
    {
        case NORTH: return (bitboard & ~Rank8Mask) <<  NORTH;
        case SOUTH: return (bitboard & ~Rank1Mask) >> -SOUTH;
        case EAST:  return (bitboard & ~FileHMask) <<  EAST;
        case WEST:  return (bitboard & ~FileAMask) >> -WEST;

        case NORTH_EAST: return (bitboard & ~FileHMask) <<  NORTH_EAST;
        case NORTH_WEST: return (bitboard & ~FileAMask) <<  NORTH_WEST;
        case SOUTH_EAST: return (bitboard & ~FileHMask) >> -SOUTH_EAST;
        case SOUTH_WEST: return (bitboard & ~FileAMask) >> -SOUTH_WEST;

        default: return 0;
    }
}

inline Bitboard getSquareMask(Square square)
{
    return squareMasks[square];
}

inline Bitboard getSquareMask(Rank rank, File file)
{
    return squareMasks[(rank * 8) + file];
}

inline Bitboard pawnAttackMask(Color color, Square square)
{
    return pawnAttacks[color][square];
}

inline Bitboard attackMask(PieceType pt, Square square)
{
    assert(pt != PAWN && withinBoard(square));
    return pseudoAttacks[pt][square];
}

inline Bitboard attackMask(PieceType pt, Square square, Bitboard blockers)
{
    assert(pt != PAWN && withinBoard(square));

    switch (pt)
    {
        case BISHOP: return bishopMagics[square].attacks[bishopMagics[square].index(blockers)];
        case ROOK  : return   rookMagics[square].attacks[  rookMagics[square].index(blockers)];
        case QUEEN : return attackMask(BISHOP, square, blockers) | attackMask(ROOK, square, blockers);
        default    : return pseudoAttacks[pt][square];
    }
}

// Gives the straight or diagonal line intersecting both given squares.
// If no such line exist, 0 is returned
inline Bitboard getLineMask(Square square, Square square2)
{
    return lineMask[square][square2];
}

// Checks if the three given squares are on the same straight or diagonal line
inline bool isAligned(Square square, Square square2, Square square3)
{
    return getLineMask(square, square2) & square3;
}

// Includes the target square but not the source
inline Bitboard getBetweenMask(Square source, Square target)
{
    assert(withinBoard(source) && withinBoard(target));
    return betweenMask[source][target];
}

#if defined(__GNUC__)  // GCC

// Returns the lsb square from the given bitboard
inline Square firstSquare(Bitboard bitboard)
{
    assert(bitboard != 0);
    return Square(__builtin_ctzll(bitboard));
}

#elif defined(_MSC_VER)

#ifdef _WIN64

// Returns the lsb square from the given bitboard
inline Square firstSquare(Bitboard bitboard)
{
    assert(bitboard);
    unsigned long square;
    _BitScanForward64(&square, bitboard);
    return (Square)square;
}

#endif

#else

#error "Compiler not supported."

#endif

// pops the lsb square from the given bitboard
inline Square popSquare(Bitboard& bitboard)
{
    Square square = firstSquare(bitboard);
    bitboard &= bitboard - 1;
    return square;
}

} // namespace ChessEngine

#endif // BITBOARD_INCLUDED