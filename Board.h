/********************************************************************
 * Board.h
 * FILE CONTENTS:
 *  Chess Board class for containing Pieces on Board
 *  Methods to work with Board
 *  Methods to generate moves
 *  Methods to move Pieces
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
#ifndef BOARD_H
#define BOARD_H
#include "Piece.h"
#include "Player.h"
#include "defs.h"
#include "myExceptions.h"
#include "mainwindow.h"

#include <cstdint>
#include <fstream>

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>

#include <assert.h>
#include <exception>
#include <stdexcept>

#define ostream std::ostream

class MainWindow;

namespace Chess{
    class Board
    {
        protected:
            class Inner;
            Inner *impl;
            MainWindow *w = NULL;
            bool keepHistory;
            friend class MainWindow;
        public:
            class PceIterator : public virtual std::iterator<std::bidirectional_iterator_tag, Piece*>{
                private:
                    friend class Board;
                    std::vector<std::vector<Piece*> >::iterator outerBegin;
                    std::vector<std::vector<Piece*> >::iterator outerCurr;
                    std::vector<Piece*>::iterator innerBegin;
                    std::vector<Piece*>::iterator innerCurr;
                public:
                    PceIterator(std::vector<std::vector<Piece*> >::iterator oit, std::vector<Piece*>::iterator iit);
                    virtual ~PceIterator() { };
                    PceIterator & operator++();
                    PceIterator & operator+(int a);
                    const bool operator<(const PceIterator &it2) const;
                    const bool operator==(const PceIterator &it2) const;
                    const bool operator!=(const PceIterator &it2) const;
                    Piece* & operator*();
            };
            friend class PceIterator;
        public:
            Board();
            Board(const Board &b);
            Board& operator=(const Board &b);
            virtual ~Board();

            int parseFen(char *fen);
            int parseMove(const std::string &mov);
            virtual int parseGame(istream &in){ return 0; }
            void addWindow(MainWindow &w);
            void updateView();

            PceIterator _begin();
            PceIterator _end();
            PceIterator findPiece(int PieceType);

            void resetBoard();

            void addWhitePlayer(Player *p);
            void addBlackPlayer(Player *p);
            Player getWhitePlayer();
            Player getBlackPlayer();

            int getSide();
            bool getGameState();
            MoveList getMoveList();
            void saveHistory(ostream &out);

            void generateAllMoves();

            /* Make and Unmake Move */
            bool makeMove(const int move);
            bool makeHistoryMove();
            void takeMove();
            /* compares Boards */
            bool operator==(const Board &b);

            std::string printBitBoard(U64 bb);
            std::string printSq(const int sq);
            std::string printMove(const int move);
            std::string printMoveList();
            std::string printGraveyard();
            /* returns text representation of the Board */
            template <typename T>
            T showBoard() const;
            friend ostream& operator<<(ostream& o, Board &game);
    };

    ostream& operator<<(ostream& o, Board &game);
}
#endif // BOARD_H
