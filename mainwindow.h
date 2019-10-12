#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QGraphicsTextItem>
#include <QInputDialog>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QKeyEvent>
#include <QProcess>
#include <QString>
#include <QObject>
#include <QTcpSocket>
#include <QtGui>

#include "qpiece.h"
#include "defs.h"
#include <assert.h>
#include <exception>
#include <stdexcept>
#include <vector>

#include "Piece.h"
#include "Board.h"
#include "ReplayBoard.h"
#include "Player.h"
#include "defs.h"
#include "FindPiece.h"
#include "myExceptions.h"
#include "qplayer.h"
#include "mytcpsocket.h"



namespace Ui {
class MainWindow;
}

namespace Chess {
class Board;
}

class MyTcpSocket;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public:
    void keyPressEvent(QKeyEvent *event);
    void addPiece(const int sq, const int type);
    void clearPiece(const int sq);
    void movePiece(const int from, const int to);
    void addBoard(Chess::Board &game);
    void setSquare(const int sq);
    void setCheck(const int sq);
    void setCheckMate(const int side);
    void setStaleMate();
    void unSet();
    void showMessage(std::string msg);
    void addWhitePlayer(QString name);
    void addBlackPlayer(QString name);
public:
    void addMove(const std::string &move, const int hisPly);
    void clearMove(const int hisPly);
    int parseMove(std::string &move);
    void setOnlineGameState(bool state);
    void flipBoard();

public:
    void initSquares();
    void initNotation();
    void resetBoard();
public slots:
    void on_lineEdit_returnPressed();
    void on_actionNew_Game_triggered();
    void on_pushButton_4_clicked();
    void on_pushButton_3_clicked();
    void on_actionLoad_Game_triggered();
    void on_pushButton_6_clicked();
    void on_pushButton_5_clicked();
    void on_actionExit_triggered();
    void on_actionSave_Game_triggered();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_actionStart_Position_triggered();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_lineEdit_cursorPositionChanged(int arg1, int arg2);
    void on_pushButton_9_clicked();

private:
    friend class Chess::Board;
    friend class MyTcpSocket;
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    std::vector<QGraphicsRectItem*> square;
    std::vector<QGraphicsPixmapItem*> notation;
    std::vector<Chess::QPiece*> piece;
    QStandardItemModel *model;
    Chess::Board *game;
    QProcess *myProcess;
    QPlayer *player[2];
    int hSq;
    int hSq1;
    int hSq2;
    int hisPly;
    QMediaPlayer *music;
    MyTcpSocket *socket;
    bool onlineGameInProgress = false;
    bool flipped = false;
};

#endif // MAINWINDOW_H
