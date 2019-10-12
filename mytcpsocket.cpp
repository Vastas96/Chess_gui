// mytcpsocket.cpp

#include "mytcpsocket.h"
#include "mainwindow.h"
#include <iostream>

MyTcpSocket::MyTcpSocket(MainWindow *w, QObject *parent) :
    QObject(parent)
{
    this->w = w;
    doConnect();
}

void MyTcpSocket::doConnect()
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    qDebug() << "connecting...";

    // this is not blocking call
    socket->connectToHost("localhost", 8080);

    // we need to wait...
    if(!socket->waitForConnected(5000))
    {
        qDebug() << "Error: " << socket->errorString();
    }
}

void MyTcpSocket::writeData(QByteArray data)
{
    socket->write(data+"\r\n");
}

int MyTcpSocket::isMyMove()
{
    return color == side;
}

int MyTcpSocket::writeMove(QByteArray data)
{
    writeData(data);
    side = side ^ 1;
    return 1;
}

int MyTcpSocket::getState()
{
    return state;
}

void MyTcpSocket::setState(int state)
{
    this->state = state;
}

void MyTcpSocket::setNickName(std::string name)
{
    nickName = name;
}

void MyTcpSocket::handle(QByteArray msg)
{
    std::string move = msg.toStdString();
    std::cout << move << std::endl;

    std::string otherUser;
    std::string color1;
    int userPos;
    int exclPos1;
    int exclPos2;

    switch (move[0]) {
    case '1':
        move = move.substr(1, 5);
        w->parseMove(move);
        side = side ^ 1;
        break;
    case 'Y':
        userPos = move.find("paired with");
        if(userPos!=std::string::npos){
            exclPos1 = move.find("@");
            otherUser = move.substr(userPos+12, exclPos1 - (userPos+12));
            exclPos2 = move.find("!");
            color1 = move.substr(exclPos2-5, 5);
            if(color1 == "white"){
                w->addBlackPlayer(QString::fromStdString(otherUser));
                w->addWhitePlayer(QString::fromStdString(nickName));
                w->showMessage("You are paired!");
                side = WHITE;
                color = WHITE;
                setState(1);
            }
            else{
                w->addBlackPlayer(QString::fromStdString(otherUser));
                w->addWhitePlayer(QString::fromStdString(nickName));
                w->flipBoard();
                w->showMessage("You are paired!");
                side = WHITE;
                color = BLACK;
                setState(1);
            }
        }
        break;
    case '7':
        break;
    default:
        break;
    }
}

void MyTcpSocket::handleSeekGame()
{

}

void MyTcpSocket::connected()
{
    qDebug() << "connected...";
    // Hey server, tell me about you.
    //socket->write("Vytenis\r\n");
}

void MyTcpSocket::disconnected()
{
    qDebug() << "disconnected...";
}

void MyTcpSocket::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written...";
}

void MyTcpSocket::readyRead()
{
    qDebug() << "reading...";

    // read the data from the socket
    while (socket->canReadLine())
    {
        QByteArray msg = QByteArray(socket->readLine());
        handle(msg);
        //qDebug() << msg;
    }
}
