#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string.h>
#include <fstream>

#include <assert.h>
#include <exception>
#include <stdexcept>

#include "Piece.h"
#include "Board.h"
#include "Player.h"
#include "defs.h"
#include "FindPiece.h"
#include "myExceptions.h"

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <fstream>

using namespace Chess;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    scene = new QGraphicsScene;
    initSquares();
    initNotation();
    for(int i = 0 ; i < 80 ; i++){
        scene->addItem(square[i]);
    }
    for(int i = 0 ; i < 16 ; i++){
        scene->addItem(notation[i]);
    }
    game = NULL;

    hSq = NO_SQ;
    hSq1 = NO_SQ;
    hSq1 = NO_SQ;

    music = new QMediaPlayer;
    music->setMedia(QUrl("qrc:/sounds/sounds/move.wav"));

    piece.insert(piece.begin(), BSIZE, nullptr);
    ui->view->setScene(scene);
    ui->view->show();

    model = new QStandardItemModel(1,2,this);

    ui->tableView->setModel(model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    setWindowTitle("Chess");
    socket = new MyTcpSocket(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(game != NULL){
        delete game;
    }
    for(int i = 0 ; i < BSIZE ; i++){
        if(square[i] != NULL){
            delete piece[i];
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << event->key();
    if(event->key() == Qt::Key_Left || event->key() == 16777233){
        //cout << "test" << endl;
        on_pushButton_3_clicked();
    }
    else if(event->key() == Qt::Key_Right || event->key() == 16777232){
        on_pushButton_4_clicked();
    }
}

void MainWindow::addPiece(const int sq, const int type)
{

    piece[sq] = new QPiece(type);
    //p[sq]->setFlag(QGraphicsItem::ItemIsMovable, true);
    if(flipped){
        piece[sq]->setY(SQ2YF(sq));
        piece[sq]->setX(SQ2XF(sq));
    }
    else{
        piece[sq]->setY(SQ2Y(sq));
        piece[sq]->setX(SQ2X(sq));
    }
    scene->addItem(piece[sq]);
    ui->view->show();
}

void MainWindow::clearPiece(const int sq)
{
    if(piece[sq] != NULL){
        delete piece[sq];
        piece[sq] = NULL;
    }
    ui->view->show();
}

void MainWindow::movePiece(const int from, const int to)
{
    if(piece[to] != NULL){
        delete piece[to];
        piece[to] = NULL;
    }
    piece[to] = piece[from];
    piece[from] = NULL;
    if(flipped){
        piece[to]->setY(SQ2YF(to));
        piece[to]->setX(SQ2XF(to));
    }
    else{
        piece[to]->setY(SQ2Y(to));
        piece[to]->setX(SQ2X(to));
    }
    music->play();
    ui->view->show();
}

void MainWindow::addBoard(Chess::Board &game)
{
    resetBoard();
    this->game = &game;
}

void MainWindow::setSquare(const int sq)
{
    square[sq]->setBrush(* new QBrush(Qt::yellow));
    if(hSq1 == NO_SQ){
        hSq1 = sq;
    }
    else{
        hSq2 = sq;
    }
}

void MainWindow::setCheck(const int sq)
{
    square[sq]->setBrush(* new QBrush(Qt::red));
    hSq = sq;
}

void MainWindow::setCheckMate(const int side)
{
    char* win[2] = {"White Win!", "Black Win!"};
    QMessageBox::information(this, tr("Check Mate!"), tr(win[side]));
}

void MainWindow::setStaleMate()
{
    QMessageBox::information(this, tr("StaleMate!"), tr("Draw!"));
}

void MainWindow::unSet()
{
    int A[3];
    A[0] = hSq;
    A[1] = hSq1;
    A[2] = hSq2;

    for(int i = 0 ; i < 3 ;i++){
        if(A[i] != NO_SQ){
            if(SQ2R(A[i]) % 2 == 0){
                if(A[i] % 2 == 0){
                    square[A[i]]->setBrush(* new QBrush(Qt::darkBlue));
                }
                else{
                    square[A[i]]->setBrush(* new QBrush(Qt::white));
                }
            }
            else if(SQ2R(A[i]) % 2 != 0){
                if(A[i] % 2 != 0){
                    square[A[i]]->setBrush(* new QBrush(Qt::darkBlue));
                }
                else{
                    square[A[i]]->setBrush(* new QBrush(Qt::white));
                }
            }
        }
    }

    hSq = NO_SQ;
    hSq1 = NO_SQ;
    hSq2 = NO_SQ;
}

void MainWindow::showMessage(string msg)
{
    QMessageBox::information(this, tr("Error"), tr(msg.c_str()));
}

void MainWindow::addWhitePlayer(QString name)
{
    if(game != NULL){
        Player *p = new Player(WHITE, name.toStdString());
        try{
            game->addWhitePlayer(p);
        }
        catch(invalid_argument &e){
            clog << e.what() << endl;
            return;
        }
    }
    else return;
    Player p = Player(WHITE, name.toStdString());
    player[WHITE] = new QPlayer(p);
    ui->playerView2->setScene(player[WHITE]);
    ui->playerView2->show();
}

void MainWindow::addBlackPlayer(QString name)
{
    if(game != NULL){
        Player *p = new Player(BLACK, name.toStdString());
        try{
            game->addBlackPlayer(p);
        }
        catch(invalid_argument &e){
            clog << e.what() << endl;
            return;
        }
    }
    else return;
    Player p = Player(BLACK, name.toStdString());
    player[BLACK] = new QPlayer(p);
    ui->playerView1->setScene(player[BLACK]);
    ui->playerView1->show();
}

void MainWindow::addMove(const string &move, const int hisPly)
{
    //cout << move << endl;
    //cout << hisPly << endl;
    this->hisPly = hisPly + 1;

    if(hisPly%2 == 0){
        model->insertRow(hisPly/2);
    }
    QModelIndex index = model->index(hisPly/2, hisPly%2, QModelIndex());
    model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
    model->setData(index, QString::fromStdString(move));

    //ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select);
    ui->tableView->setCurrentIndex(index);
}

void MainWindow::clearMove(const int hisPly)
{
    this->hisPly = hisPly;
    if(hisPly%2 == 0){
        model->removeRow(hisPly/2);
    }
    QModelIndex index = model->index(hisPly/2, hisPly%2, QModelIndex());
    model->setData(index,"");
    //ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select);
    ui->tableView->setCurrentIndex(model->index((hisPly - 1)/2, (hisPly - 1)%2, QModelIndex()));
}

int MainWindow::parseMove(string &move)
{
    if(game != NULL){
        try{
            if(move.size() < 4){
                return false;
            }
            int parsed_move = game->parseMove(&move[0]);
            return game->makeMove(parsed_move);
        }
        catch(IllegalMove &e){
            return false;
        }
    }
    ui->lineEdit->clear();
    return false;
}

void MainWindow::setOnlineGameState(bool state)
{
    onlineGameInProgress = state;
}

void MainWindow::flipBoard()
{
    for(int i = 0 ; i < BSIZE ; i++){
        square[i]->setRect(420 - (SQ2F(i))*60,(SQ2R(i))*60,60,60);
    }

    for(int i = 0 ; i < BSIZE ; i++){
        if(piece[i] != NULL){
            piece[i]->setX( SQ2XF(i) );
            piece[i]->setY( SQ2YF(i) );
        }
    }
    for(int i = 0 ; i < 8 ; i++){
        notation[i]->setY(483);
        notation[i]->setX( 420 - i * 60);
    }
    for(int i = 8 ; i < 16 ; i++){
        notation[i]->setY( 420 - (i - 8) * 60);
        notation[i]->setX(483);
    }
    ui->view->setScene(scene);
    ui->view->show();
    flipped = true;
}

void MainWindow::initSquares()
{
    for(int i = 0 ; i < BSIZE ; i++){
        square.push_back(new QGraphicsRectItem);
        square[i]->setRect((SQ2F(i))*60,(7-(SQ2R(i)))*60,60,60);
        if(SQ2R(i) % 2 == 0){
            if(i % 2 == 0){
                square[i]->setBrush(* new QBrush(Qt::darkBlue));
            }
        }
        else if(SQ2R(i) % 2 != 0){
            if(i % 2 != 0){
                square[i]->setBrush(* new QBrush(Qt::darkBlue));
            }
        }
    }

    for(int i = 64 ; i < 72 ; i++){
        square.push_back(new QGraphicsRectItem);
        square[i]->setRect(((i - 64) * 60), 483, 60, 30);
    }
    for(int i = 72 ; i < 80 ; i++){
        square.push_back(new QGraphicsRectItem);
        square[i]->setRect(483, ((i - 72) * 60), 30, 60);
    }

}

void MainWindow::initNotation()
{
    for(int i = 0 ; i < 16 ; i++){
        notation.push_back(new QGraphicsPixmapItem);
    }

    notation[0]->setPixmap(QPixmap(":/images/img/FA.png"));
    notation[1]->setPixmap(QPixmap(":/images/img/FB.png"));
    notation[2]->setPixmap(QPixmap(":/images/img/FC.png"));
    notation[3]->setPixmap(QPixmap(":/images/img/FD.png"));
    notation[4]->setPixmap(QPixmap(":/images/img/FE.png"));
    notation[5]->setPixmap(QPixmap(":/images/img/FF.png"));
    notation[6]->setPixmap(QPixmap(":/images/img/FG.png"));
    notation[7]->setPixmap(QPixmap(":/images/img/FH.png"));
    notation[8]->setPixmap(QPixmap(":/images/img/R8.png"));
    notation[9]->setPixmap(QPixmap(":/images/img/R7.png"));
    notation[10]->setPixmap(QPixmap(":/images/img/R6.png"));
    notation[11]->setPixmap(QPixmap(":/images/img/R5.png"));
    notation[12]->setPixmap(QPixmap(":/images/img/R4.png"));
    notation[13]->setPixmap(QPixmap(":/images/img/R3.png"));
    notation[14]->setPixmap(QPixmap(":/images/img/R2.png"));
    notation[15]->setPixmap(QPixmap(":/images/img/R1.png"));

    for(int i = 0 ; i < 8 ; i++){
        notation[i]->setY(483);
        notation[i]->setX(i * 60);
    }
    for(int i = 8 ; i < 16 ; i++){
        notation[i]->setY( (i - 8) * 60);
        notation[i]->setX(483);
    }
}


void MainWindow::on_lineEdit_returnPressed()
{
    if(game != NULL){
        std::string input = (ui->lineEdit->text()).toUtf8().constData();
        try{
            if(socket->isMyMove()){
                if(game->makeMove(game->parseMove(&input[0]))){
                    socket->writeMove("1"+ui->lineEdit->text().toUtf8());
                }
            }
            else{
               showMessage("It's not your move!");
            }
        }
        catch(IllegalMove &e){
            QMessageBox::information(this, tr("Error"), tr(e.what()));
            std::clog << e.what()<< ": " << e.getInput() << " on move " << e.getHisPly() << std::endl;
        }
        catch(...){

        }

    ui->lineEdit->clear();
    }
}

void MainWindow::resetBoard()
{
    if(game != NULL){
        delete game;
        delete model;

        model = new QStandardItemModel(1,2,this);
        ui->tableView->setModel(model);

        hisPly = 0;

        for(int i = 0 ; i < BSIZE ; i++){
            if(piece[i] != NULL){
                delete piece[i];
                piece[i] = NULL;
            }
        }
    }
}

void MainWindow::on_actionNew_Game_triggered()
{
    resetBoard();
    game = new Board;
    game->addWindow(*this);
    game->parseFen(START_FEN);
}

void MainWindow::on_pushButton_4_clicked()
{
    if(game != NULL){
        if(!hisPly) hisPly--;
        game->takeMove();
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    if(game != NULL){
        hisPly++;
        game->makeHistoryMove();
    }
}

void MainWindow::on_actionLoad_Game_triggered()
{
    string fileName;
    QString qFileName = QFileDialog::getOpenFileName(
                this,
                tr("Select Game"),
                "C:\\Users\\Vytenis\\build-GUI-Desktop_Qt_5_7_0_MSVC2013_64bit-Debug",
                "All files (*.*);;Text File (*.txt)"
                );
    if(!qFileName.isEmpty()){
        fileName = qFileName.toStdString();
    }
    else return;
    resetBoard();
    game = new ReplayBoard;
    game->addWindow(*this);
    game->parseFen(START_FEN);
    fstream data;
    data.open(fileName,ios::in);
    try{
        game->parseGame(data);
        this->hisPly = 0;
    }
    catch(IllegalMove &e){
        clog << e.what()<< ": " << e.getInput() << " on move " << e.getHisPly() << std::endl;
        QMessageBox::information(this, tr("Error"), tr("Error Loading Game. Check log"));
    }
    data.close();
}

void MainWindow::on_pushButton_6_clicked()
{
    bool ok;
    QInputDialog *input = new QInputDialog;
    input->setOptions(QInputDialog::NoButtons);

    QString text =  input->getText(NULL ,"Enter White Player's Name",
                                          "Name:", QLineEdit::Normal,
                                          QDir::home().dirName(), &ok);

     if (ok && !text.isEmpty()){
        addWhitePlayer(text);
     }
}

void MainWindow::on_pushButton_5_clicked()
{
    bool ok;
    QInputDialog *input = new QInputDialog;
    input->setOptions(QInputDialog::NoButtons);

    QString text =  input->getText(NULL ,"Enter Black Player's Name",
                                          "Name:", QLineEdit::Normal,
                                          QDir::home().dirName(), &ok);

     if (ok && !text.isEmpty()){
        addBlackPlayer(text);
     }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionSave_Game_triggered()
{
    string fileName;
    if(game != NULL){
        QString qFileName = QFileDialog::getSaveFileName(
                    this,
                    tr("Select Game"),
                    "game",
                    "Text File (*.txt)"
                    );
        if(!qFileName.isEmpty()){
            fileName = qFileName.toStdString();
            ofstream out(fileName);
            game->saveHistory(out);
            out.close();
        }
        else return;
    }
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    int ply = 2 * index.row() + index.column() + 1;
    //cout << ply << " " << hisPly << endl;
    if(ply == hisPly) return;
    if(ply < hisPly){
        int diff = hisPly - ply;
        hisPly -= diff;
        for(int i = 0 ; i < diff ; i++){
            game->takeMove();
        }
    }
    else{
        int diff = ply - hisPly;
        hisPly += diff;
        for(int i = 0 ; i < diff ; i++){
            //cout << "miau " << endl;
            game->makeHistoryMove();
        }
    }
    ui->tableView->setCurrentIndex(model->index((hisPly-1)/2, (hisPly-1)%2, QModelIndex()));
}

void MainWindow::on_actionStart_Position_triggered()
{
    bool ok;
    QInputDialog *input = new QInputDialog;
    input->setOptions(QInputDialog::NoButtons);

    QString text =  input->getText(NULL ,"Parse Fen",
                                          "Enter Fen:", QLineEdit::Normal,
                                          START_FEN, &ok);
    //socket->writeData(text.toUtf8());
     if (ok && !text.isEmpty()){
        resetBoard();
        game = new Board;
        game->addWindow(*this);
        try{
            game->parseFen(&(text.toStdString())[0]);
        }
        catch(FenException &e){
            QMessageBox::information(this, tr("Error"), tr("Error Parsing Game. Check log"));
            clog << e.what() << endl;
            resetBoard();
        }
        catch(invalid_argument &e){
            QMessageBox::information(this, tr("Error"), tr("Error Parsing Game. Check log"));
            clog << e.what() << endl;
            resetBoard();
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    bool ok;
    QInputDialog *input = new QInputDialog;
    input->setOptions(QInputDialog::NoButtons);

    on_actionNew_Game_triggered();
    setOnlineGameState(true);
    QString text =  input->getText(NULL ,"Enter Your Name",
                                          "Name:", QLineEdit::Normal,
                                          QDir::home().dirName(), &ok);

    socket->writeData(text.toUtf8());
    socket->setNickName(text.toStdString());
    socket->writeData("2");
    socket->setState(2);
}

void MainWindow::on_pushButton_clicked()
{
    /*
    if(chessClient->isGame() == false){
        chessClient->sendData("3");
        while(true){
            try{
                chessClient->recieveData();
            }
            catch(int i){
                showMessage("Failed to join game!");
                return;
            }

            catch(char c){
                on_actionNew_Game_triggered();
                setOnlineGameState(true);
                on_pushButton_5_clicked();
                showMessage("Joined game! Wait for opponent move!");
                chessClient->setGame(true);
                while(true){
                    if (parseMove(chessClient->recieveData())){
                        return;
                    }
                    QCoreApplication::processEvents();
                    //QTest::qSleep(100);
                }
            }
        }
    }
    else{
        showMessage("You already play!");
        return;
    }
    */
}

void MainWindow::on_pushButton_7_clicked()
{
    /*
    delete chessClient;
    try{
        chessClient = new client2(this);
        showMessage("Connected to server!");
    }
    catch(...){
        showMessage("Failed to connect to server!");
    }
    */
}

void MainWindow::on_pushButton_8_clicked()
{
    //delete chessClient;
}

void MainWindow::on_lineEdit_cursorPositionChanged(int arg1, int arg2)
{

}

void MainWindow::on_pushButton_9_clicked()
{
    flipBoard();
}
