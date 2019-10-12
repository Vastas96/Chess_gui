/********************************************************************
 * Player.h
 * FILE CONTENTS:
 *  Chess Player class for interacting with Board
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"
#include <string>
#include "Printable.h"

#define ostream std::ostream
#define istream std::istream

namespace Chess{
    class Player : public Printable
    {
        private:
            int side;
            std::string name;
            static int playerCount;
        public:
            friend class Board;
            Player();
            Player(int color, std::string name);
            Player(int color);
            ~Player();

            void setSide(int color);
            void setName(std::string name);

            /* Did not need after all */
            /*
            void select(int sq, Board &A);
            void select(std::string xy, Board &A);
            */

            int getColor();
            std::string getName();
            static int getPlayerCount();
            std::string toString() const;

            friend istream& operator>>(istream& i, Player &p);
    };
    istream& operator>>(istream& i, Player &p);
}
#endif // PLAYER_H
