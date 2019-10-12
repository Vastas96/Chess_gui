/********************************************************************
 * Piece.h
 * FILE CONTENTS:
 *  Chess Piece class for storing data about Piece
 *  Methods to work with Chess Pieces
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
#include "Piece.h"
#include "defs.h"

#include <sstream>
#include <iostream>

#include <assert.h>
#include <exception>
#include <stdexcept>

using namespace std;

namespace Chess{
    int Piece::pieceCount = 0;
    int Piece::instanceCount = 0;


    void Piece::setID(int instanceCount){
        ID = instanceCount;
    }
    void Piece::setSide(int color){
        if(color >= 0 && color < 2){
            this->side = color;
        }
        else{
            #ifdef DEBUG
                clog << "invalid color: " << color << endl;
            #endif // DEBUG
            throw invalid_argument("Invalid side!");
        }
    }
    //Possible types: wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK
    void Piece::setType(int type){
        if(type > EMPTY && type <= bK){
            this->type = type;
        }
        else{
            this->type = wP;
            #ifdef DEBUG
                clog << "invalid piece type: " << type << endl;
            #endif // DEBUG
            throw invalid_argument("Invalid piece type! Set Pawn as default");
        }
    }

    Piece::Piece(){
        #ifdef DEBUG
            clog << "Default Piece constructor called!" << endl;
        #endif // DEBUG
        pieceCount++;
    }

    Piece::Piece(const Piece &p){
        #ifdef DEBUG
            clog << "Piece copy constructor called!" << endl;
        #endif // DEBUG
        this->ID = p.ID;
        this->side = p.side;
        this->type = p.type;
        pieceCount++;
    }

    Piece& Piece::operator=(Piece &p){
        #ifdef DEBUG
            clog << "Piece operator= called!" << endl;
        #endif // DEBUG
        pieceCount++;
        mySwap(*this, p);
        return *this;
    }
    Piece::Piece(int color, int intType){
        setID(instanceCount);
        setSide(color);
        setType(intType);
        pieceCount++;
        instanceCount++;
    }
    Piece::~Piece(){
        pieceCount--;
        assert(pieceCount >= 0);
    }
    const int Piece::getID()const{
        #ifdef DEBUG
            clog << "getID(" << ") called!" << endl;
        #endif // DEBUG
        return ID;
    }
    const int Piece::getColor()const{
        return side;
    }
    const int Piece::getType() const{
        return type;
    }

    string Piece::toString() const{
        stringstream ss;
        string pieceName[] = {"P", "N", "B", "R", "Q", "K", "P", "N", "B", "R", "Q", "K"};
        string color[] = {"w", "b"};
        ss << color[getColor()] << pieceName[getType() - 1];
        return ss.str();
    }

    int Piece::getPieceCount(){
        return pieceCount;
    }
    bool operator<(Piece& a, Piece& b){
        return a.getType() < b.getType();
    }

    void mySwap(Piece& a, Piece& b){ //nothrow
        swap(a.ID, b.ID);
        swap(a.side, b.side);
        swap(a.type, b.type);
    }
    ostream& operator<<(ostream& o, Piece &z){
        o << z.toString();
        return o;
    }

    istream& operator>>(istream& i, Piece &z){
        Piece p = z;
        cout << p << endl;
        try{
            int color;
            int intType;
            {
                i >> color;
                i.clear();
                i >> intType;
                i.clear();
            }
            if(i.fail()){
                i.clear();
                i.ignore(256, '\n');
                throw ios_base::failure("Input error");
            }
            z.setSide(color);
            z.setType(intType);
        }
        catch(...){
            z = p;
            throw;
        }
    }
}
