#ifndef FINDPIECE_H
#define FINDPIECE_H

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>

#include "Piece.h"
#include "Player.h"
#include "defs.h"

namespace Chess{
    class FindPiece : public std::unary_function<Piece*,bool>
    {
        public:
            int PieceType;
            FindPiece(int PieceType){
                this->PieceType = PieceType;
            }
            virtual ~FindPiece(){ }
            bool operator()(Piece* p) const {
                if(p != NULL){
                    if(p->getType() == PieceType){
                        return true;
                    }
                }
                return false;
            }
    };

    class PcePtrCmp{
        public:
            bool operator()(Piece* a, Piece* b){
                return *a < *b;
            }
    };

    template<typename T>
    class MySort
    {
        public:
            bool operator()(const T &a, const T &b) const{
                return a < b;
            }
    };
    template<typename T>
    class MyTransform
    {
        public:
            void operator()(T &a){
                a+=3;
            }
    };
}
#endif // FINDPIECE_H
