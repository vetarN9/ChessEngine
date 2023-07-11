#ifndef BITBOARD_INCLUDED
#define BITBOARD_INCLUDED

#include <stdint.h>
#include <cassert>

#include "defs.h"

namespace ChessEngine {

namespace Bitboard {

void init();
void print(uint64_t bitBoard);

} // namespace Bitboard

constexpr uint64_t Rank1Mask = 0xFF;
constexpr uint64_t Rank2Mask = Rank1Mask << (8 * 1);
constexpr uint64_t Rank3Mask = Rank1Mask << (8 * 2);
constexpr uint64_t Rank4Mask = Rank1Mask << (8 * 3);
constexpr uint64_t Rank5Mask = Rank1Mask << (8 * 4);
constexpr uint64_t Rank6Mask = Rank1Mask << (8 * 5);
constexpr uint64_t Rank7Mask = Rank1Mask << (8 * 6);
constexpr uint64_t Rank8Mask = Rank1Mask << (8 * 7);

constexpr uint64_t FileAMask = 0x0101010101010101;
constexpr uint64_t FileBMask = FileAMask << 1;
constexpr uint64_t FileCMask = FileAMask << 2;
constexpr uint64_t FileDMask = FileAMask << 3;
constexpr uint64_t FileEMask = FileAMask << 4;
constexpr uint64_t FileFMask = FileAMask << 5;
constexpr uint64_t FileGMask = FileAMask << 6;
constexpr uint64_t FileHMask = FileAMask << 7;

constexpr uint64_t boardEdgeMask = Rank1Mask | Rank8Mask | FileAMask | FileHMask;

extern uint64_t squareMasks[NUM_SQUARES];
extern uint64_t pawnAttacks[NUM_COLORS][NUM_SQUARES];
extern uint64_t pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

struct Magic 
{
    uint64_t* attacks;
    uint64_t  mask;
    uint64_t  magic;
    uint32_t shift;

    uint32_t index(uint64_t blockers) const
    {
        return ((blockers & mask) * magic) >> shift;
    }
};

extern Magic bishopMagics[NUM_SQUARES];
extern Magic rookMagics[NUM_SQUARES];

constexpr uint64_t getFileMask(int square) 
{
    return FileAMask << File(square & 0b111);
}

constexpr bool withinBoard(int square)
{
    return square >= A1 && square < NUM_SQUARES;
}

constexpr bool withinBoard(int square, int dir)
{
    switch (dir)
    {
    case EAST: return squareMasks[square] & ~FILE_H;
    case WEST: return squareMasks[square] & ~FILE_A;
    }

    int destination = square + dir;
    return destination >= A1 && destination < NUM_SQUARES;
}

constexpr uint64_t shift(uint64_t bitBoard, Direction dir)
{
    switch (dir)
    {
    case NORTH: return (bitBoard & ~Rank8Mask) <<  NORTH;
    case SOUTH: return (bitBoard & ~Rank1Mask) >> -SOUTH;
    case EAST:  return (bitBoard & ~FileHMask) <<  EAST;
    case WEST:  return (bitBoard & ~FileAMask) >> -WEST;

    case NORTH_EAST: return (bitBoard & ~FileHMask) <<  NORTH_EAST;
    case NORTH_WEST: return (bitBoard & ~FileAMask) <<  NORTH_WEST;
    case SOUTH_EAST: return (bitBoard & ~FileHMask) >> -SOUTH_EAST;
    case SOUTH_WEST: return (bitBoard & ~FileAMask) >> -SOUTH_WEST;
    }

    return 0;
}

constexpr uint64_t pawnAttackMask(uint64_t pawn, Color color)
{
    return (color == WHITE) ? shift(pawn, NORTH_WEST) | shift(pawn, NORTH_EAST)
                            : shift(pawn, SOUTH_WEST) | shift(pawn, SOUTH_EAST);
}

/* inline uint64_t pawnAttackMask(Square square, Color color)
{
  return pawnAttacks[color][square];
} */

constexpr uint64_t kingAttackMask(uint64_t king)
{
    return shift(king, NORTH) | shift(king, NORTH_EAST) |
           shift(king,  EAST) | shift(king, SOUTH_EAST) |
           shift(king, SOUTH) | shift(king, SOUTH_WEST) |
           shift(king,  WEST) | shift(king, NORTH_WEST);
}

constexpr uint64_t knightAttackMask(uint64_t knight)
{
    return shift(shift(knight, NORTH), NORTH_EAST) | shift(shift(knight, NORTH), NORTH_WEST) |
           shift(shift(knight, SOUTH), SOUTH_EAST) | shift(shift(knight, SOUTH), SOUTH_WEST) |
           shift(shift(knight, EAST),  NORTH_EAST) | shift(shift(knight,  EAST), SOUTH_EAST) |
           shift(shift(knight, WEST),  NORTH_WEST) | shift(shift(knight,  WEST), SOUTH_WEST);
}

inline uint64_t getAttackMask(PieceType pieceType, int square, uint64_t blockers)
{

  assert(pieceType != PAWN && withinBoard(square));

  switch (pieceType)
  {
  case BISHOP: return bishopMagics[square].attacks[bishopMagics[square].index(blockers)];
  case ROOK  : return   rookMagics[square].attacks[  rookMagics[square].index(blockers)];
  case QUEEN : return getAttackMask(BISHOP, square, blockers) | getAttackMask(ROOK, square, blockers);
  default    : return pseudoAttacks[pieceType][square];
  }
}



inline uint64_t getSquareMask(Square square)
{
    return squareMasks[square];
}

inline uint64_t getSquareMask(Rank rank, File file)
{
    return squareMasks[(rank * 8) + file];
}

inline uint64_t popBit(uint64_t bitboard, int square)
{
    return (bitboard & (1ULL << square)) ? bitboard ^= (1ULL << square) : 0;
}

inline int numBits(uint64_t bitboard)
{
    int numBits = 0;

    while (bitboard)
    {
        bitboard &= bitboard - 1;
        numBits++;
    }
    
    return numBits;
}

inline Square lsbSquare(uint64_t bitboard)
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

inline uint64_t random64()
{
    return ((uint64_t)((random32()) & 0xFFFF) <<  0) | 
           ((uint64_t)((random32()) & 0xFFFF) << 16) |
           ((uint64_t)((random32()) & 0xFFFF) << 32) | 
           ((uint64_t)((random32()) & 0xFFFF) << 48);
}

inline uint64_t random64FewBits() 
{
  return random64() & random64() & random64();
}

constexpr uint64_t getRankMask(int square) 
{
    return Rank1Mask << (8 * Rank(square >> 3));
}

} // namespace ChessEngine

#endif // BITBOARD_INCLUDED