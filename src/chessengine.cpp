#include <iostream>

// temp
#include <chrono>
#include <bitset> 

#include "position.h"
#include "bitboard.h"

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

    pos.init();
    pos.placePiece(WHITE_PAWN, C4);
    pos.print();
    pos.placePiece(BLACK_KNIGHT, E6);
    pos.placePiece(WHITE_KNIGHT, A6);
    pos.placePiece(BLACK_QUEEN, B6);
    pos.placePiece(WHITE_KING, C6);
    pos.print();
    pos.removePiece(A6);
    pos.removePiece(E6);
    pos.print();
    pos.movePiece(C6, C3);
    pos.movePiece(C4, H8);
    pos.print();

    /* for (int square = A1; square < NUM_SQUARES; square++)
    {
        Bitboard::print(getAttackMask(QUEEN, square, squareMasks[D4] | squareMasks[D5] | squareMasks[E4] | squareMasks[E5]));
    } */
    
}