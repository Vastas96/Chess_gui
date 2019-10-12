/********************************************************************
 * Piece.h
 * FILE CONTENTS:
 *  Chess Piece class for storing data about Piece
 *  Methods to work with Chess Pieces
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
#ifndef PIECE_H
#define PIECE_H
#include <string>

#include "defs.h"
#include "Printable.h"

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>


#define ostream std::ostream
#define istream std::istream

namespace Chess{

    class Piece : public Printable
    {
        protected:
            int ID;
            int side;
            int type;
            static int pieceCount;
            static int instanceCount;
        public:
            Piece();
            Piece(const Piece &p);
            Piece& operator=(Piece &p);
            Piece(int color, int intType);
            virtual ~Piece();

            void setID(int instanceCount);
            void setSide(int color);
            void setType(int intType);

            const int getID()const;
            const int getColor()const;
            const int getType()const;

            //Returns text representation of color and Piece type
            std::string toString() const;

            static int getPieceCount();

            const bool operator<(const Piece &b) const{
                return (this->getType() < b.getType());
            }
            friend void mySwap(Piece& a, Piece& b);
            friend istream& operator>>(istream& i, Piece &z);
    };
    void mySwap(Piece& a, Piece& b);
    istream& operator>>(istream& i, Piece &z);
}

#endif // PIECE_H
