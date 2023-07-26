#include <iostream>

#include "defs.h"
#include "position.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"

using namespace ChessEngine;

int main(int argc, char* argv[])
{
    Bitboards::init();
 
    Position pos;
    PosInfo posInfo;

    pos.Set(startPosFEN, &posInfo);
    //pos.Set("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &posInfo);

    pos.Print();

    Perft::go(pos, 6);
}