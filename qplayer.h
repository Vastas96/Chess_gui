#ifndef QPLAYER_H
#define QPLAYER_H
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <Player.h>


class QPlayer : public QGraphicsScene, public Chess::Player
{
public:
    QPlayer();
    QPlayer(Chess::Player &p);
protected:
    QGraphicsTextItem *playerName;
};

#endif // QPLAYER_H
