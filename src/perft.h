#ifndef PEFT_INCLUDED
#define PEFT_INCLUDED

#include "defs.h"
#include "position.h"

namespace ChessEngine {

namespace Perft {

void go(Position& pos, int depth);
uint64_t getNodes(Position& pos, int depth);

} // namespace Perft

} // namespace ChessEngine

#endif // PERFT_INCLUDED