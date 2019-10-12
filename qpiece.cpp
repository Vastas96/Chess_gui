#include "qpiece.h"
#include <QGraphicsPixmapItem>
#include <iostream>

namespace Chess {

    using namespace std;

    QPiece::QPiece()
    {
        Piece::type = 1;
        initMap();
        //cout << Piece::type << endl;
        //cout << pieceMap[Piece::type] << endl;
        setPixmap(QPixmap(pieceMap[Piece::type]));
    }

    QPiece::QPiece(int type)
    {
        initMap();
        Piece::type = type;
        setPixmap(QPixmap(pieceMap[Piece::type]));
    }

    void QPiece::initMap()
    {
        /*
        string dir = ":/images/1";
        for(int i = 1 ; i < 13 ; i++){
            dir[9]++;
            pieceMap.insert(pair<int,string>(i,dir));
        }
        */
        pieceMap.insert(pair<int,QString>(1,":/images/img/1.png"));
        pieceMap.insert(pair<int,QString>(2,":/images/img/2.png"));
        pieceMap.insert(pair<int,QString>(3,":/images/img/3.png"));
        pieceMap.insert(pair<int,QString>(4,":/images/img/4.png"));
        pieceMap.insert(pair<int,QString>(5,":/images/img/5.png"));
        pieceMap.insert(pair<int,QString>(6,":/images/img/6.png"));
        pieceMap.insert(pair<int,QString>(7,":/images/img/7.png"));
        pieceMap.insert(pair<int,QString>(8,":/images/img/8.png"));
        pieceMap.insert(pair<int,QString>(9,":/images/img/9.png"));
        pieceMap.insert(pair<int,QString>(10,":/images/img/10.png"));
        pieceMap.insert(pair<int,QString>(11,":/images/img/11.png"));
        pieceMap.insert(pair<int,QString>(12,":/images/img/12.png"));
    }
}
