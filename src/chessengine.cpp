#include <iostream>

// temp
#include <chrono>
#include <bitset> 

#include "position.h"
#include "bitboard.h"
#include "defs.h"

using namespace ChessEngine;

using namespace std::chrono;

int main(int argc, char* argv[])
{
    auto start = high_resolution_clock::now();
 
    Bitboard::init();
 
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
 
    std::cout << duration.count() << " milliseconds" << std::endl;

    Position pos;

    pos.setPosFromFEN(startPosFEN);
    pos.print();

    //Bitboard::print(getAttackMask(KNIGHT, square, pos.getPieceMask()));

    /* for (int square = A1; square < NUM_SQUARES; square++)
    {
        Bitboard::print(getAttackMask(QUEEN, square, squareMasks[D4] | squareMasks[D5] | squareMasks[E4] | squareMasks[E5]));
    } */
    
}