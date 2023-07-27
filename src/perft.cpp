#include <iostream>
#include <chrono>

#include "perft.h"
#include "movegen.h"

namespace ChessEngine {

namespace Perft {

namespace {  // anonymous namespace

uint64_t perft(Position& pos, int depth, bool isRoot = false);
std::string getString(Move move);

} // anonymous namespace

void go(Position& pos, int depth)
{
    assert(depth >= 0);

    std::cout << "Running performance test\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    uint64_t nodes = perft(pos, depth, true);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "\nDepth: " << depth << "\n";
    std::cout << "Nodes: "   << nodes << "\n";
    std::cout << "Time: "    << duration.count() << " milliseconds\n" << std::endl;
}

uint64_t getNodes(Position& pos, int depth)
{
    return perft(pos, depth);
}

namespace {  // anonymous namespace

uint64_t perft(Position& pos, int depth, bool isRoot /*= false*/)
{
    // Special case when 0 depth
    if (depth == 0)
        return 1;

    MoveList moveList;
    PosInfo posInfo;
    uint64_t nodes = 0;

    // The recursive escape condition
    bool isLeaf = (depth == 2);

    MoveGen::generate(pos, moveList);

    for (int i = 0; i < moveList.count; i++)
    {
        uint64_t count;
        Move move = moveList.moves[i].move;

        pos.MakeMove(move, posInfo);

        if (isLeaf)
        {
            MoveList moveListLeaf;
            MoveGen::generate(pos, moveListLeaf);
            count = moveListLeaf.count;
        }

        else 
            count = perft(pos, depth - 1);

        pos.UndoMove(move);
        nodes += count;

        if (isRoot)
            std::cout << "    " << getString(move) << ": " << count << "\n";
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