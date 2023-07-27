#include <iostream>

#include "defs.h"
#include "position.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "test.h"

using namespace ChessEngine;

int main(int argc, char* argv[])
{
    Bitboards::init();
    Test::perft();
}