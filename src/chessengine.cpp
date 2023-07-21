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
    //pos.Set("4r3/8/8/qk6/5p2/6p1/PPPPPPPP/RNBQKBNR w Kq - 0 49");
    //pos.Set("8/8/kn4nn/P2PpPPP/6P1/8/6P1/RNBQKBNR w KQkq e6 0 1");
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