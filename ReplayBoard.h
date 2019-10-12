#ifndef REPLAYBOARD_H
#define REPLAYBOARD_H

#include "Board.h"

#include <cstdint>
#include <fstream>

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>

#include <assert.h>
#include <exception>
#include <stdexcept>

//class Chess::Board;

namespace Chess{

    class ReplayBoard : public Chess::Board{
        protected:
            class ReplayInner;
        public:
            ReplayBoard();
            ReplayBoard(const Board &b);
            virtual ~ReplayBoard() { }
            bool makeMove(const int move) = delete;
            int parseGame(istream &in);
            void otherImpl();
    };
}
#endif // REPLAYBOARD_H
