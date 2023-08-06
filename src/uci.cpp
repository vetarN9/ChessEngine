#include <iostream>
#include <sstream>

#include "uci.h"
#include "defs.h"
#include "position.h"
#include "movegen.h"

namespace ChessEngine {

void UCI::loop()
{
    Position pos;
    PosInfo posInfo;
    MoveList moveList;

    std::string token, line;

    pos.Set(startPosFEN, &posInfo);

    std::istringstream iss;

    while (true)
    {
        // Wait for input on cin
        if (!getline(std::cin, line))
            break;

        iss.str(line);
        std::string token;

        // Tokenize the input string
        while (iss >> token)
        {
/*             if (token == "uci")
                sendEngineInfo();

            else if (token == "isready")
                sendReadyOk();

            else if (token == "position")
                parsePosition(iss, pos);

            else if (token == "go")
                parseGo(iss, pos);

            else if (token == "stop")
                stopSearch();

            else if (token == "quit")
                return; */
        }

        // Clear the stringstream for the next iteration
        iss.clear();
    }
    
}

} // namespace ChessEngine