/********************************************************************
 * Player.cpp
 * FILE CONTENTS:
 *  Chess Player class for interacting with Board
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
#include "Player.h"
#include "defs.h"

#include <sstream>
#include <iostream>

#include <assert.h>
#include <exception>
#include <stdexcept>

using namespace std;

namespace Chess{

    int Player::playerCount = 0;

    void Player::setSide(int color){
        if(color >= 0 && color < 2){
            this->side = color;
        }
    }
    void Player::setName(string name){
        this->name = name;
    }
    Player::Player(){
        playerCount++;
    }
    Player::Player(int color, string name){
        setSide(color);
        setName(name);
        playerCount++;
    }
    Player::Player(int color){
        setSide(color);
        if(playerCount == 0){
            setName("Player1");
        }
        else{
            setName("Player2");
        }
        playerCount++;
    }
    Player::~Player() { };

    /* Did not need */
    /*
    void Player::select(int sq, Board &A){
        #ifdef DEBUG
            clog << ":Player::select(" << ") called!" << endl;
        #endif // DEBUG
        if(sq >= 0 && sq < 64){
            A.select(sq, isWhite);
        }
        else{
            #ifdef DEBUG
                clog << "invalid coordinates: " << sq << endl;
            #endif // DEBUG
            throw invalid_argument("Failed to select, invalid coordinates!");
        }
    }
    //Converts standard chess coordinates
    void Player::select(string xy, Board &A){
        if(xy.length() == 2 && xy[0] >= 'A' && xy[0] <= 'H' && xy[1] >= '1' && xy[1] <= '8'){
            A.select(xy[0]-'A' + (xy[1] - '1') * 7, isWhite);
        }
        else{
            #ifdef DEBUG
                clog << "invalid coordinates: " << xy << endl;
            #endif // DEBUG
            throw invalid_argument("Failed to select, invalid coordinates!");
        }
    }
    */
    int Player::getColor(){
        return side;
    }
    string Player::getName(){
        return name;
    }
    int Player::getPlayerCount(){
        return playerCount;
    }

    string Player::toString() const{
        stringstream ss;
        string color[] = {"White", "Black"};
        ss << color[side] << " " << name;

        return ss.str();

    }

    /*
    ostream& Player::operator<<(ostream& o){
        o << this->toString();
        return o;
    }
    */
    istream& operator>>(istream& i, Player &p){
        int color;
        string name;
        {
            i >> color;
            i.clear();
            i >> name;
        }
        if(i.fail()){
            i.clear();
            i.ignore(256, '\n');
            throw ios_base::failure("Input error");
        }
        p.setSide(color);
        p.setName(name);
    }
}
