#ifndef QPIECE_H
#define QPIECE_H
#include <QGraphicsPixmapItem>
#include "Piece.h"
#include <QString>
#include <iterator>
#include <map>

namespace Chess {

    class QPiece : public QGraphicsPixmapItem, public Piece
    {
    protected:
        std::map<int,QString> pieceMap;
        void initMap();
    public:
        QPiece();
        QPiece(int type);
    };
}
#endif // QPIECE_H
