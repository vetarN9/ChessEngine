#include <iostream>

// temp
#include <chrono>
#include <bitset> 

#include "defs.h"
#include "position.h"
#include "bitboard.h"
#include "movegen.h"

using namespace ChessEngine;

using namespace std::chrono;

int main(int argc, char* argv[])
{
    auto start = high_resolution_clock::now();
 
    Bitboards::init();
 
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
 
    std::cout << duration.count() << " milliseconds" << std::endl;

    Position pos;

    pos.Set(startPosFEN);
    //pos.Set("8/5k1p/1p1pRpN1/pP1q4/P1P3Qp/5NbP/rrr1P1K1/B7 w Kq a6 0 49");
    //pos.Set("k7/8/3P4/2K4q/6P1/8/PPPPP1PP/6b1 w Kq - 0 49");
    pos.Set("1Bb3BN/R2Pk2r/1Q5B/4q2R/2bN4/4Q1BK/1p6/1bq1R1rb w - - 0 1");
    pos.Print();
    //Bitboards::print(pos.Checkers());
    MoveList m;
    generateMoves(pos, m);
    std::cout << m.count << std::endl;

    /* Bitboard pinners = pos.Pinners(BLACK);
    Bitboards::print(pos.SliderBlockers(pos.Pieces(), G2, pinners));
    Bitboards::print(pinners); */

    //Bitboards::print(attackMask(KNIGHT, square, pos.getPieceMask()));

    /* for (Square square = A1; square < NUM_SQUARES; square++)
    {
        Bitboards::print(attackMask(QUEEN, square, squareMasks[D4] | squareMasks[D5] | squareMasks[E4] | squareMasks[E5]));
    }
     */
}