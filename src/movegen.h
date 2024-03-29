#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "defs.h"
#include "position.h"

namespace ChessEngine {

enum GenType
{
  ALL,
  CAPTURES,
  QUIETS,
  EVASIONS
};

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

namespace MoveGen {

// Generates all the legal moves for the given position and populates the given moveList
void generate(const Position& pos, MoveList& moveList, GenType genType = ALL);

} // namespace MoveGen

} // namespace ChessEngine

#endif // MOVEGEN_INCLUDED