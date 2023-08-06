#ifndef UCI_INCLUDED
#define UCI_INCLUDED

namespace ChessEngine {

class Position;

namespace UCI {

// The main loop of the engine. Waits for commands
// on stdin and executes the corresponding function
void loop();

} // namespace UCI

} // namespace ChessEngine

#endif // UCI_INCLUDED