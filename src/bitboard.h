#ifndef BITBOARD_INCLUDED
#define BITBOARD_INCLUDED

#include <stdint.h>
#include <cassert>

#include "defs.h"

namespace ChessEngine {

namespace Bitboards {

void init();
void print(Bitboard bitboard);

} // namespace Bitboards

constexpr Bitboard Rank1Mask = 0xFF;
constexpr Bitboard Rank2Mask = Rank1Mask << (8 * 1);
constexpr Bitboard Rank3Mask = Rank1Mask << (8 * 2);
constexpr Bitboard Rank4Mask = Rank1Mask << (8 * 3);
constexpr Bitboard Rank5Mask = Rank1Mask << (8 * 4);
constexpr Bitboard Rank6Mask = Rank1Mask << (8 * 5);
constexpr Bitboard Rank7Mask = Rank1Mask << (8 * 6);
constexpr Bitboard Rank8Mask = Rank1Mask << (8 * 7);

constexpr Bitboard FileAMask = 0x0101010101010101;
constexpr Bitboard FileBMask = FileAMask << 1;
constexpr Bitboard FileCMask = FileAMask << 2;
constexpr Bitboard FileDMask = FileAMask << 3;
constexpr Bitboard FileEMask = FileAMask << 4;
constexpr Bitboard FileFMask = FileAMask << 5;
constexpr Bitboard FileGMask = FileAMask << 6;
constexpr Bitboard FileHMask = FileAMask << 7;

constexpr Bitboard boardEdgeMask = Rank1Mask | Rank8Mask | FileAMask | FileHMask;

extern Bitboard squareMasks[NUM_SQUARES];
extern Bitboard lineMask[NUM_SQUARES][NUM_SQUARES];
extern Bitboard betweenMask[NUM_SQUARES][NUM_SQUARES];
extern Bitboard pawnAttacks[NUM_COLORS][NUM_SQUARES];
extern Bitboard pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

struct Magic 
{
    Bitboard* attacks;
    Bitboard  mask;
    Bitboard  magic;
    uint32_t shift;

    uint32_t index(Bitboard blockers) const { return ((blockers & mask) * magic) >> shift; }
};

extern Magic bishopMagics[NUM_SQUARES];
extern Magic rookMagics[NUM_SQUARES];

inline Bitboard getSquareMask(Square square) { return squareMasks[square]; }

//Operators for modifying a bitboard with a square
inline Bitboard  operator& (Bitboard  bitboard, Square square) { return bitboard &  getSquareMask(square); }
inline Bitboard  operator| (Bitboard  bitboard, Square square) { return bitboard |  getSquareMask(square); }
inline Bitboard  operator^ (Bitboard  bitboard, Square square) { return bitboard ^  getSquareMask(square); }
inline Bitboard& operator|=(Bitboard& bitboard, Square square) { return bitboard |= getSquareMask(square); }
inline Bitboard& operator^=(Bitboard& bitboard, Square square) { return bitboard ^= getSquareMask(square); }

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

constexpr bool withinBoard(Square square)
{
    return square >= A1 && square <= H8;
}

constexpr bool withinBoard(Square square, Direction dir)
{
    switch (dir)
    {
        case EAST: return squareMasks[square] & ~FILE_H;
        case WEST: return squareMasks[square] & ~FILE_A;
        default:   break;
    }

    Square dest = square + dir;
    return dest >= A1 && dest <= H8;
}

constexpr Bitboard shift(Bitboard bitboard, Direction dir)
{
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

constexpr Bitboard pawnAttackMask(Bitboard pawn, Color color)
{
    return (color == WHITE) ? shift(pawn, NORTH_WEST) | shift(pawn, NORTH_EAST)
                            : shift(pawn, SOUTH_WEST) | shift(pawn, SOUTH_EAST);
}

constexpr Bitboard kingAttackMask(Bitboard king)
{
    return shift(king, NORTH) | shift(king, NORTH_EAST) |
           shift(king,  EAST) | shift(king, SOUTH_EAST) |
           shift(king, SOUTH) | shift(king, SOUTH_WEST) |
           shift(king,  WEST) | shift(king, NORTH_WEST);
}

constexpr Bitboard knightAttackMask(Bitboard knight)
{
    return shift(shift(knight, NORTH), NORTH_EAST) | shift(shift(knight, NORTH), NORTH_WEST) |
           shift(shift(knight, SOUTH), SOUTH_EAST) | shift(shift(knight, SOUTH), SOUTH_WEST) |
           shift(shift(knight, EAST),  NORTH_EAST) | shift(shift(knight,  EAST), SOUTH_EAST) |
           shift(shift(knight, WEST),  NORTH_WEST) | shift(shift(knight,  WEST), SOUTH_WEST);
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

inline Bitboard getSquareMask(Rank rank, File file)
{
    return squareMasks[(rank * 8) + file];
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

inline Bitboard popBit(Bitboard bitboard, Square square)
{
    return (bitboard & (1ULL << square)) ? bitboard ^= (1ULL << square) : 0;
}

inline int numBits(Bitboard bitboard)
{
    int numBits = 0;

    while (bitboard)
    {
        bitboard &= bitboard - 1;
        numBits++;
    }
    
    return numBits;
}

inline Square lsbSquare(Bitboard bitboard)
{
    assert(bitboard);
    return Square(numBits((bitboard & -bitboard) - 1));
}

inline uint32_t random32()
{
    static uint32_t random = 1804289383;
    
    // XOR shift algorithm
    random ^= random << 13;
    random ^= random >> 17;
    random ^= random << 5;
    
    return random;
}

inline Bitboard random64()
{
    return ((Bitboard)((random32()) & 0xFFFF) << 0)  |
           ((Bitboard)((random32()) & 0xFFFF) << 16) |
           ((Bitboard)((random32()) & 0xFFFF) << 32) |
           ((Bitboard)((random32()) & 0xFFFF) << 48);
}

inline Bitboard random64FewBits() 
{
    return random64() & random64() & random64();
}


#if defined(__GNUC__)  // GCC

// Returns the lsb square from the given bitboard
inline Square getSquare(Bitboard bitboard)
{
    assert(bitboard != 0);
    return Square(__builtin_ctzll(bitboard));
}

inline Square msb(Bitboard bitboard)
{
    assert(bitboard != 0);
    return Square(63 ^ __builtin_clzll(bitboard));
}

#elif defined(_MSC_VER)

#ifdef _WIN64

// Returns the lsb square from the given bitboard
inline Square getSquare(Bitboard bitboard)
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
    Square square = getSquare(bitboard);
    bitboard &= bitboard - 1;
    return square;
}

} // namespace ChessEngine

#endif // BITBOARD_INCLUDED