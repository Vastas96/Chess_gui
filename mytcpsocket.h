#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDebug>

#include "mainwindow.h"

class MainWindow;

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(MainWindow *w, QObject *parent = 0);

    void doConnect();
    void writeData(QByteArray data);
    int isMyMove();
    int writeMove(QByteArray data);
    int getState();
    void setState(int state);
    void setNickName(std::string name);
private:
    void handle(QByteArray msg);
    void handleSeekGame();
signals:

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    QTcpSocket *socket;
    int state = 0;
    int side = BOTH;
    int color = BOTH;
    MainWindow *w;
    std::string nickName;
};

#endif // MYTCPSOCKET_H
