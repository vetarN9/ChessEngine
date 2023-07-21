#include <iostream>
#include <bitset>

#include "bitboard.h"

namespace ChessEngine {

Bitboard squareMasks[NUM_SQUARES];
Bitboard lineMask[NUM_SQUARES][NUM_SQUARES];
Bitboard betweenMask[NUM_SQUARES][NUM_SQUARES];
Bitboard pawnAttacks[NUM_COLORS][NUM_SQUARES];
Bitboard pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

Magic bishopMagics[NUM_SQUARES];
Magic rookMagics[NUM_SQUARES];

namespace {

Bitboard bishopAttackTable[5248];
Bitboard rookAttackTable[102400];

void initMagics(PieceType pt, Magic magics[], Bitboard attackTable[]);
Bitboard slidingAttack(PieceType pt, Square attackerSquare, Bitboard blockers);

} // anonymous namespace


// Initializes pre-calculated attack tables using magic bitboards
void Bitboards::init()
{
    // Init square mask
    for (Square square = A1; square < NUM_SQUARES; square++)
        squareMasks[square] = 1ULL << square;

    // Init between mask
    
    initMagics(BISHOP, bishopMagics, bishopAttackTable);
    initMagics(ROOK, rookMagics, rookAttackTable);

    // Init pseudo attack tables
    for (Square sq = A1; sq < NUM_SQUARES; sq++)
    {
        pawnAttacks[WHITE][sq]    = pawnAttackMask(squareMasks[sq], WHITE);
        pawnAttacks[BLACK][sq]    = pawnAttackMask(squareMasks[sq], BLACK);
        pseudoAttacks[KING][sq]   = kingAttackMask(squareMasks[sq]);
        pseudoAttacks[KNIGHT][sq] = knightAttackMask(squareMasks[sq]);
        pseudoAttacks[BISHOP][sq] = attackMask(BISHOP, sq, 0);
        pseudoAttacks[ROOK][sq]   = attackMask(ROOK,   sq, 0);
        pseudoAttacks[QUEEN][sq]  = pseudoAttacks[BISHOP][sq] | pseudoAttacks[ROOK][sq];

        // Init line and between masks
        for (Square sq2 = A1; sq2 < NUM_SQUARES; sq2++)
        {
            for (PieceType pt : { BISHOP, ROOK })
            {
                if (pseudoAttacks[pt][sq] & sq2)
                {
                    lineMask[sq][sq2]    = (attackMask(pt, sq, 0) & attackMask(pt, sq2, 0)) | sq | sq2;
                    betweenMask[sq][sq2] = (attackMask(pt, sq, getSquareMask(sq2)) & attackMask(pt, sq2, getSquareMask(sq)));
                }

                betweenMask[sq][sq2] |= sq2;
            }
        }
    }
}

// Prints the given bitboard to stdout
void Bitboards::print(Bitboard bitboard)
{
    std::cout << "    bitboard: " << bitboard << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+---+\n";

    for (Rank rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (File file = FILE_A; file < NUM_FILES; file++)
            std::cout << ((bitboard & getSquareMask(rank, file)) ? "| X " : "|   ");

        std::cout << "| \n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n" << std::endl;
}

namespace {

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
            for (++count, i = 0; i < size; ++i)
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
        int square = attackerSquare;

        while (shift(squareMasks[square], dir) && !(squareMasks[square] & blockers))
            attacks |= squareMasks[square += dir];
    }

    return attacks;
}

} // anonymous namespace

} // namespace ChessEngine