#include <iostream>
#include <bitset>

#include "bitboard.h"

namespace ChessEngine {

uint64_t squareMasks[NUM_SQUARES];
uint64_t pawnAttacks[NUM_COLORS][NUM_SQUARES];
uint64_t pseudoAttacks[NUM_PIECE_TYPES][NUM_SQUARES];

Magic bishopMagics[NUM_SQUARES];
Magic rookMagics[NUM_SQUARES];

namespace {

uint64_t bishopAttackTable[5248];
uint64_t rookAttackTable[102400];

void initMagics(PieceType pieceType, Magic magics[], uint64_t attackTable[]);

} // anonymous namespace


void Bitboard::init()
{
    for (int square = A1; square < NUM_SQUARES; square++)
        squareMasks[square] = 1ULL << square;
    
    initMagics(BISHOP, bishopMagics, bishopAttackTable);
    initMagics(ROOK, rookMagics, rookAttackTable);

    for (int square = A1; square < NUM_SQUARES; square++)
    {
        pseudoAttacks[KING][square]   = kingAttackMask(squareMasks[square]);
        pseudoAttacks[KNIGHT][square] = knightAttackMask(squareMasks[square]);
        pseudoAttacks[BISHOP][square] = getAttackMask(BISHOP, square, 0);
        pseudoAttacks[ROOK][square]   = getAttackMask(ROOK,   square, 0);
        pseudoAttacks[QUEEN][square]  = pseudoAttacks[BISHOP][square] | pseudoAttacks[ROOK][square];

        pawnAttacks[WHITE][square] = pawnAttackMask(squareMasks[square], WHITE);
        pawnAttacks[BLACK][square] = pawnAttackMask(squareMasks[square], BLACK);
    }
}

// Prints the given bitboard to stdout
void Bitboard::print(uint64_t bitboard)
{
    std::cout << "        " << bitboard << std::endl;
    std::cout << "  +---+---+---+---+---+---+---+---+\n";

    for (int rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (int file = FILE_A; file < NUM_FILES; file++)
        {
            std::cout << ((bitboard & getSquareMask(Rank(rank), File(file))) ? "| X " : "|   ");
        }

        std::cout << "| \n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    
    std::cout << "    a   b   c   d   e   f   g   h" << std::endl;
}

namespace {

    // Returns a bitmask for all squares that the given sliding piece attacks, counting up until
    // the board edge or a blocker from the given blockers.
    uint64_t slidingAttack(PieceType pieceType, int attackerSquare, uint64_t blockers)
    {
        uint64_t attacks = 0;

        Direction plus[]   = {NORTH, SOUTH, EAST, WEST};
        Direction cross[]  = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

        for (Direction dir : (pieceType == BISHOP ? cross : plus))
        {
            int square = attackerSquare;
            while (shift(squareMasks[square], dir) && !(squareMasks[square] & blockers))
                attacks |= squareMasks[square += dir];
        }

        return attacks;
    }

    // Precalculates all bishop and rook attacks and uses the fancy magic bitboard
    // technique to make a lookup with a board state to see where the sliding piece
    // attacks. For reference, see: https://www.chessprogramming.org/Magic_Bitboards
    void initMagics(PieceType pieceType, Magic magics[], uint64_t attackTable[])
    {
        uint64_t blockers[4096], reference[4096], edgeMask;
        int epoch[4096] = {}, size = 0, count = 0;

        for (int square = A1; square < NUM_SQUARES; square++)
        {
            // Alias for the curent magic
            Magic& magic = magics[square];

            edgeMask = ((Rank1Mask | Rank8Mask) & ~getRankMask(square)) | ((FileAMask | FileHMask) & ~getFileMask(square));

            // Init magic members
            magic.mask = slidingAttack(pieceType, square, 0) & ~edgeMask;
            magic.shift = NUM_SQUARES - numBits(magic.mask);
            magic.attacks = (square == A1) ? attackTable : magics[square-1].attacks + size;

            // Carry-Rippler trick to enumerate all permutations of blockers that can
            // block the piece. 
            // https://www.chessprogramming.org/Traversing_Subsets_of_a_Set
            uint64_t bitboard = 0;
            size = 0;
            do
            {
                reference[size] = slidingAttack(pieceType, square, bitboard);
                blockers[size] = bitboard;
                bitboard = (bitboard - magic.mask) & magic.mask;
                size++;
            } while (bitboard);

            // Brute force to find a magic number that maps every permutation of blockers 
            // to an index that looks up the correct sliding attack in the attackTable.
            for (int i = 0; i < size;)
            {
                magic.magic = random64FewBits();

                // Verify that the magic number maps each blockers to an index that 
                // looks up the correct sliding attack
                for (++count, i = 0; i < size; ++i)
                {
                    uint32_t index = magic.index(blockers[i]);

                    if (epoch[index] < count)
                    {
                        epoch[index] = count;
                        magic.attacks[index] = reference[i];
                    }
                    else if (magic.attacks[index] != reference[i]) // magic number did not work
                        break;
                }   
            }
        }
    }
} // anonymous namespace

} // namespace ChessEngine