#include <iostream>
#include <chrono>

#include "perft.h"
#include "movegen.h"

namespace ChessEngine {

namespace Perft {

namespace {  // anonymous namespace

uint64_t perft(Position& pos, int depth);
std::string getString(Move move);

} // anonymous namespace

void go(Position& pos, int depth)
{
    std::cout << "Running performance test\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    MoveList moveList;
    generateMoves(pos, moveList);

    PosInfo posInfo;

    uint64_t nodes = 0;

    if (depth == 0)
        nodes = 1;

    else 
    {
        for (int i = 0; i < moveList.count; i++)
        {
            pos.MakeMove(moveList.moves[i].move, posInfo);
            uint64_t count = perft(pos, depth - 1);
            pos.UndoMove(moveList.moves[i].move);

            nodes += count;

            std::cout << "    " << getString(moveList.moves[i].move) << ": " << count << "\n";
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "\nDepth: " << depth << "\n";
    std::cout << "Nodes: "   << nodes << "\n";
    std::cout << "Time: "    << duration.count() << " milliseconds\n" << std::endl;
}

namespace {  // anonymous namespace

uint64_t perft(Position& pos, int depth)
{
    if (depth == 0)
        return 1;

    MoveList moveList;
    generateMoves(pos, moveList);

    PosInfo posInfo;

    uint64_t nodes = 0;

    for (int i = 0; i < moveList.count; i++)
    {
        pos.MakeMove(moveList.moves[i].move, posInfo);

        nodes += perft(pos, depth - 1);

        pos.UndoMove(moveList.moves[i].move);
    }

    return nodes;
}

std::string getString(Move move)
{
    Square from = getFromSquare(move);
    Square to   = getToSquare(move);

    std::string moveStr = algebraicNotation(from) + algebraicNotation(to);

    if (getMoveType(move) == PROMOTION)
        moveStr += " pnbrqk"[getPromotionType(move)];

    return moveStr;
}

} // anonymous namespace

} // namespace Perft

} // namespace ChessEngine