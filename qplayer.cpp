#include "qplayer.h"

QPlayer::QPlayer()
{

}

QPlayer::QPlayer(Chess::Player &p)
{
    QString qstr = QString::fromStdString(p.getName());
    playerName = new QGraphicsTextItem(qstr);
    playerName->setScale(2);
    addItem(playerName);
    //addItem(new QGraphicsTextItem(qstr));
}
