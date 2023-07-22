#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "defs.h"
#include "position.h"

namespace ChessEngine {

struct MoveData
{
    Move move;
    int score;
};

struct MoveList
{
    MoveData moves[MAX_MOVES] = {};
    int count = 0;
};

enum GenType {
  EVASIONS,
  NON_EVASIONS,
};

void generateMoves(const Position& pos, MoveList& moveList);

} // namespace ChessEngine

#endif // MOVEGEN_INCLUDED