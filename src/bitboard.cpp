#include <iostream>

#include "bitboard.h"

namespace ChessEngine {

Bitboard squareMasks[NUM_SQUARES];
Bitboard lineMask[NUM_SQUARES][NUM_SQUARES];
Bitboard betweenMask[NUM_SQUARES][NUM_SQUARES];
Bitboard pawnAttacks[NUM_COLORS][NUM_SQUARES];
Bitboard pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

Magic bishopMagics[NUM_SQUARES];
Magic rookMagics[NUM_SQUARES];

namespace {  // anonymous namespace

Bitboard bishopAttackTable[5248];
Bitboard rookAttackTable[102400];

void initMagics(PieceType pt, Magic magics[], Bitboard attackTable[]);
Bitboard slidingAttack(PieceType pt, Square attackerSquare, Bitboard blockers);
inline int numBits(Bitboard bitboard);

inline void initAttackTables(Square sq);
inline void initLineAndBetweenMasks(Square from);

constexpr Bitboard pawnAttackMask(Bitboard pawn, Color color);
constexpr Bitboard kingAttackMask(Bitboard king);
constexpr Bitboard knightAttackMask(Bitboard knight);

inline uint32_t random32();
inline Bitboard random64();
inline Bitboard random64FewBits();

} // anonymous namespace

void Bitboards::init()
{
    // Init square masks
    for (Square sq = A1; sq < NUM_SQUARES; sq++)
        squareMasks[sq] = 1ULL << sq;
    
    initMagics(BISHOP, bishopMagics, bishopAttackTable);
    initMagics(ROOK, rookMagics, rookAttackTable);

    for (Square sq = A1; sq < NUM_SQUARES; sq++)
    {
        initAttackTables(sq);
        initLineAndBetweenMasks(sq);
    }
}

void Bitboards::print(Bitboard bitboard)
{
    std::cout << "    bitboard: " << bitboard << "\n";
    std::cout << "  +---+---+---+---+---+---+---+---+\n";

    for (Rank rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (File file = FILE_A; file < NUM_FILES; file++)
            std::cout << ((bitboard & getSquareMask(rank, file)) ? "| X " : "|   ");

        std::cout << "| \n  +---+---+---+---+---+---+---+---+\n";
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n" << std::endl;
}

namespace {  // anonymous namespace

// Precalculates all bishop and rook attacks and uses the fancy magic bitboard
// technique to make a lookup with a board state to see where the sliding piece
// attacks. For reference, see: https://www.chessprogramming.org/Magic_Bitboards
void initMagics(PieceType pt, Magic magics[], Bitboard attackTable[])
{
    Bitboard blockers[4096], reference[4096], edgeMask;
    int epoch[4096] = {}, size = 0, count = 0;

    for (Square square = A1; square < NUM_SQUARES; square++)
    {
        // Alias for the curent magic
        Magic& m = magics[square];

        edgeMask = ((Rank1Mask | Rank8Mask) & ~getRankMask(square)) |
                   ((FileAMask | FileHMask) & ~getFileMask(square));

        // Init magic members
        m.mask = slidingAttack(pt, square, 0) & ~edgeMask;
        m.shift = NUM_SQUARES - numBits(m.mask);
        m.attacks = (square == A1) ? attackTable : magics[square-1].attacks + size;

        // Carry-Rippler trick to enumerate all permutations of blockers that can
        // block the piece. https://www.chessprogramming.org/Traversing_Subsets_of_a_Set
        Bitboard bitboard = 0;
        size = 0;
        do
        {
            reference[size] = slidingAttack(pt, square, bitboard);
            blockers[size] = bitboard;
            bitboard = (bitboard - m.mask) & m.mask;
            size++;
        } while (bitboard);

        // Brute force to find a magic number that maps every permutation of blockers 
        // to an index that looks up the correct sliding attack in the attackTable.
        for (int i = 0; i < size;)
        {
            m.magic = random64FewBits();

            // Verify that the magic number maps each blockers to an index that 
            // looks up the correct sliding attack
            for (count++, i = 0; i < size; i++)
            {
                uint32_t index = m.index(blockers[i]);

                if (epoch[index] < count)
                {
                    epoch[index] = count;
                    m.attacks[index] = reference[i];
                }

                else if (m.attacks[index] != reference[i]) // magic number did not work
                    break;
            }   
        }
    }
}

// Returns a bitmask for all squares that the given sliding piece attacks, counting up until
// the board edge or a blocker from the given blockers.
Bitboard slidingAttack(PieceType pt, Square attackerSquare, Bitboard blockers)
{
    Bitboard attacks = 0;

    Direction plus[]   = {NORTH, SOUTH, EAST, WEST};
    Direction cross[]  = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    for (Direction dir : (pt == BISHOP ? cross : plus))
    {
        Square sq = attackerSquare;

        while (shift(squareMasks[sq], dir) && !(squareMasks[sq] & blockers))
            attacks |= squareMasks[sq += dir];
    }

    return attacks;
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

inline void initAttackTables(Square sq)
{
    pawnAttacks[WHITE][sq]    = pawnAttackMask(squareMasks[sq], WHITE);
    pawnAttacks[BLACK][sq]    = pawnAttackMask(squareMasks[sq], BLACK);
    pseudoAttacks[KING][sq]   = kingAttackMask(squareMasks[sq]);
    pseudoAttacks[KNIGHT][sq] = knightAttackMask(squareMasks[sq]);
    pseudoAttacks[BISHOP][sq] = attackMask(BISHOP, sq, 0);
    pseudoAttacks[ROOK][sq]   = attackMask(ROOK,   sq, 0);
    pseudoAttacks[QUEEN][sq]  = pseudoAttacks[BISHOP][sq] | pseudoAttacks[ROOK][sq];
}

inline void initLineAndBetweenMasks(Square from)
{
    for (Square to = A1; to < NUM_SQUARES; to++)
    {
        for (PieceType pt : { BISHOP, ROOK })
        {
            if (pseudoAttacks[pt][from] & to)
            {
                lineMask[from][to]    = (attackMask(pt, from, 0) & attackMask(pt, to, 0)) | from | to;
                betweenMask[from][to] = (attackMask(pt, from, getSquareMask(to)) & attackMask(pt, to, getSquareMask(from)));
            }

            // Also add the destination square
            betweenMask[from][to] |= to;
        }
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

} // anonymous namespace

} // namespace ChessEngine