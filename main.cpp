#include "mainwindow.h"
#include "Board.h"
#include "ReplayBoard.h"
#include <QApplication>


using namespace std;
using namespace Chess;

//i.velza@gmail.com

int main(int argc, char *argv[])
{
    streambuf *clog_backup = clog.rdbuf();
    streambuf *cerr_backup = cerr.rdbuf();

    fstream logFile;
    streampos init_pos;
    logFile.open("log.txt", fstream::out);

    clog.rdbuf(logFile.rdbuf());
    cerr.rdbuf(logFile.rdbuf());

    QApplication a(argc, argv);
    Piece *p;
    /*
    {
        ReplayBoard *b = new ReplayBoard;
        b->parseFen(START_FEN);
        cout << *b << endl;
        MainWindow m;
        //m.on_actionNew_Game_triggered();
        cout << p->getPieceCount() << endl;
        delete b;
    }
    */
    cout << p->getPieceCount() << endl;
    MainWindow w;

    w.show();


    return a.exec();

}
