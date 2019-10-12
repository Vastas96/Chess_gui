/********************************************************************
 * Board.cpp
 * FILE CONTENTS:
 *  Chess Board class for containing Pieces on Board
 *  Methods to work with Board
 *  Methods to generate moves
 *  Methods to move Pieces
 * AUTHOR:
 *  Vytenis Sabaliauskas
********************************************************************/
//#include "Board.h"
#include "ReplayBoard.h"
#include "Player.h"
#include "defs.h"
#include "FindPiece.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>

#include <assert.h>
#include <exception>
#include <stdexcept>

namespace Chess{
    using namespace std;

    class Board::Inner{
        protected:
            friend class Board;
            friend class ReplayBoard;
            vector<vector<Piece*> > square;
            multiset<Piece*, PcePtrCmp> graveyard;
            Player *white = NULL;
            Player *black = NULL;
            MoveList mlist;
            vector<Undo> history;

            int kingSq[2];
            int pceNum[13];
            int pList[13][10];
            int side;
            bool isGame;
            int enPas;
            int fiftyMove;
            int ply;
            int hisPly;
            int castlePerm;

            U64 posKey;
            U64 pieceKeys[13][64];
            U64 sideKey;
            U64 castleKeys[16];

            void addWhitePlayer(Player *p);
            void addBlackPlayer(Player *p);
            void setGame(bool game);
            void setSide(int side);
            bool sqAttacked(const int sq, const int side);
            bool isCheck();
            bool isCheckMate();
            bool isStaleMate();
            bool isSqOnBoard(const int sq);

            void initBitMasks();
            void initHashKeys();
            void updateListsMaterial();
            U64 generatePosKey();

            void addQuietMove(const int move);
            void addCaptureMove(const int move);
            void addEnPassantMove(const int move);
            void addWhitePawnCapMove(const int from, const int to, const int cap);
            void addWhitePawnMove(const int from, const int to);
            void addBlackPawnCapMove(const int from, const int to, const int cap);
            void addBlackPawnMove(const int from, const int to);
            void generateWhitePawnMoves(const int sq);
            void generateBlackPawnMoves(const int sq);
            void generateCastling();
            void generateSlidingPieceMoves(const int sq);
            void generateNonSlidingPieceMoves(const int sq);
            void generateAllMoves();

            void clearPiece(const int sq);
            void addPiece(const int sq, const int pce);
            void movePiece(const int from, const int to);
            bool makeMove(const int move);
            virtual bool makeHistoryMove();
            virtual void takeMove();

            void cleanUp();

        public:
            Inner();
            Inner(const Inner &other);
            virtual ~Inner();
    };

    Board::PceIterator::PceIterator(vector<vector<Piece*> >::iterator oit, vector<Piece*>::iterator iit)
    : outerBegin(oit), outerCurr(oit), innerBegin(iit), innerCurr(iit)
    {

    }
    Board::PceIterator& Board::PceIterator::operator++(){
        if(--(outerCurr->end()) == innerCurr){
            ++outerCurr;
            innerCurr = outerCurr->begin();
        }
        else{
            ++innerCurr;
        }
        return *this;
    }
    Board::PceIterator & Board::PceIterator::operator+(int a){
        for(int i = 0 ; i < a ; i++){
            operator++();
        }
        return *this;
    }
    const bool Board::PceIterator::operator<(const PceIterator &it2) const{
       if(this->outerCurr < it2.outerCurr){
            return true;
        }
        if(this->innerCurr < it2.innerCurr){
            return true;
        }
        return false;
    }
    const bool Board::PceIterator::operator==(const PceIterator &it2) const{
        if(this->outerCurr != it2.outerCurr){
            return false;
        }
        if(this->innerCurr != it2.innerCurr){
            return false;
        }
        return true;
    }

    const bool Board::PceIterator::operator!=(const PceIterator &it2) const{
        return !(*this == it2);
    }
    Piece* & Board::PceIterator::operator*(){
        return *innerCurr;
    }

    Board::PceIterator Board::_begin(){
        return PceIterator(impl->square.begin(), (impl->square.begin())->begin());
    }
    Board::PceIterator Board::_end(){
        return PceIterator(--(impl->square.end()), ((--(impl->square.end()))->end()));
    }

    Board::Inner::Inner() : square(){
        initHashKeys();
        vector<Piece*> v;
        v.insert(v.begin(), FSIZE, nullptr);
        square.insert(square.begin(), RSIZE, v);
        //white = new Player(WHITE, "Vytenis");
        //black = new Player(BLACK, "Matas");
    }
    Board::Inner::Inner(const Inner &other){
        vector<Piece*> v;
        v.insert(v.begin(), FSIZE, nullptr);
        square.insert(square.begin(), RSIZE, v);

        for(int i = 0 ; i < FSIZE ; i++){
            for(int j = 0 ; j < RSIZE ; j++){
                if(other.square[i][j] != NULL){
                    square[i][j] = new Piece(*other.square[i][j]);
                }
            }
        }
        if(other.white != NULL){
            white = new Player(*other.white);
        }
        if(other.black != NULL){
            black = new Player(*other.black);
        }
        mlist = other.mlist;
        history = other.history;

        for(int i = 0 ; i < 2 ; i++){
            kingSq[i] = other.kingSq[i];
        }
        for(int i = 0 ; i < 13 ; i++){
            pceNum[i] = other.pceNum[i];
        }
        for(int i = 0 ; i < 13 ; i++){
            for(int j = 0 ; j < 10 ; j++){
                pList[i][j] = other.pList[i][j];
            }
        }
        side = other.side;
        isGame = other.isGame;
        enPas = other.enPas;
        fiftyMove = other.fiftyMove;
        ply = other.ply;
        hisPly = other.hisPly;
        castlePerm = other.castlePerm;

        posKey = other.posKey;
        initHashKeys();
    }
    Board::Inner::~Inner(){
        cleanUp();
    }

    void Board::Inner::addWhitePlayer(Player *p){
        if(white == NULL){
            white = p;
        }
        else throw invalid_argument("Can't add more Players");
    }
    void Board::Inner::addBlackPlayer(Player *p){
        if(black == NULL){
            black = p;
        }
        else throw invalid_argument("Can't add more Players");
    }

    void Board::Inner::setGame(bool game){
        isGame = game;
    }
    void Board::Inner::setSide(int side){
        if(side >= 0 || side <= 1){
            this->side = side;
        }
        else{
            #ifdef DEBUG
                clog << "invalid side type: " << side << endl;
            #endif // DEBUG
            throw invalid_argument("Failed to setSide, invalid side type!");
        }
    }

    Board::Board() : impl(new Inner()) {
        resetBoard();
        keepHistory = false;
        #ifdef DEBUG
            clog << "Board constructor called!" << endl;
        #endif // DEBUG

    }
    Board::Board(const Board &b)
        :impl(new Inner(*(b.impl)))
    {
        resetBoard();
        #ifdef DEBUG
            clog << "Board copy constructor called!" << endl;
        #endif // DEBUG
    }
    Board& Board::operator=(const Board &b){

        if(&b != this){
            delete impl;
        }
        impl = new Inner(*(b.impl));
        return *this;
        #ifdef DEBUG
            clog << "Board operator= called!" << endl;
        #endif // DEBUG
    }
    Board::~Board(){
        delete impl;
    }

    void Board::Inner::initHashKeys(){
        srand(5415);
        for(int i = 0 ; i < 13 ; i++) {
            for(int j = 0; j < 64; j++) {
                pieceKeys[i][j] = RAND_64;
            }
        }
        sideKey = RAND_64;
        for(int i = 0 ; i < 16 ; i++) {
            castleKeys[i] = RAND_64;
        }
    }
    void Board::Inner::updateListsMaterial(){
        int piece;
        int kingCount = 0;
        for(int i = 0 ; i < BSIZE ; ++i){
            if(square[SQ2F(i)][SQ2R(i)] != NULL){
                piece = square[SQ2F(i)][SQ2R(i)]->getType();
                pList[piece][pceNum[piece]] = i;
                pceNum[piece]++;
                if(piece == wK){
                    kingSq[WHITE] = i;
                    kingCount++;
                }
                if(piece == bK){
                    kingSq[BLACK] = i;
                    kingCount++;
                }
            }
        }
        assert(kingCount == 2);
    }
    U64 Board::Inner::generatePosKey(){
        U64 finalKey = 0;
        int piece = EMPTY;
        // pieces
        for(int i = 0 ; i < BSIZE; i++){
            if(square[SQ2F(i)][SQ2R(i)] != NULL){
                piece = square[SQ2F(i)][SQ2R(i)]->getType();
            }
            finalKey ^= pieceKeys[piece][i];
        }
        if(side == WHITE){
            finalKey ^= sideKey;
        }
        if(enPas != NO_SQ){
            assert(enPas >= 0 && enPas < BSIZE);
            finalKey ^= pieceKeys[EMPTY][enPas];
        }
        assert(castlePerm >= 0 && castlePerm < 16);
        finalKey ^= castleKeys[castlePerm];
        return finalKey;
    }

    int Board::parseFen(char *fen){
        try{
            //cout << fen << endl;
            if(fen == NULL){
                throw EmptyFen();
            }
            int  r = RANK_8;
            int  f = FILE_A;
            int  piece = 0;
            int  c = 0;
            resetBoard();
            while ((r >= RANK_1) && *fen) {
                c = 1;
                switch (*fen) {
                    case 'p': piece = bP; break;
                    case 'r': piece = bR; break;
                    case 'n': piece = bN; break;
                    case 'b': piece = bB; break;
                    case 'k': piece = bK; break;
                    case 'q': piece = bQ; break;
                    case 'P': piece = wP; break;
                    case 'R': piece = wR; break;
                    case 'N': piece = wN; break;
                    case 'B': piece = wB; break;
                    case 'K': piece = wK; break;
                    case 'Q': piece = wQ; break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                        piece = EMPTY;
                        c = *fen - '0';
                        break;
                    case '/':
                    case ' ':
                        r--;
                        f = FILE_A;
                        fen++;
                        continue;
                    default:
                        throw FenException(fen,"Invalid argument in Fen");
                }
                assert(f>=FILE_A && f <= FILE_H);
                assert(r>=RANK_1 && r <= RANK_8);
                for (int i = 0 ; i < c ; i++) {
                    if (piece != EMPTY) {
                        impl->square[f][r] = new Piece(piece/7, piece);
                    }
                    f++;
                }
                fen++;
            }
            if(*fen != 'w' && *fen != 'b'){
                throw invalid_argument("Side to move not given in Fen");
            }
            impl->side = (*fen == 'w') ? WHITE : BLACK;
            fen += 2;
            for (int i = 0 ; i < 4 ; i++) {
                if (*fen == ' ') {
                    break;
                }
                switch(*fen) {
                    case 'K': impl->castlePerm |= WKCA; break;
                    case 'Q': impl->castlePerm |= WQCA; break;
                    case 'k': impl->castlePerm |= BKCA; break;
                    case 'q': impl->castlePerm |= BQCA; break;
                    default:	     break;
                }
                fen++;
            }
            fen++;
            assert(impl->castlePerm>=0 && impl->castlePerm <= 15);
            if (*fen != '-') {
                f = fen[0] - 'a';
                r = fen[1] - '1';
                assert(f>=FILE_A && f <= FILE_H);
                assert(r>=RANK_1 && r <= RANK_8);
                impl->enPas = FR2SQ(f,r);
            }
            impl->posKey = impl->generatePosKey();
            impl->updateListsMaterial();
            updateView();
            return 0;
        }
        catch(FenException &e){
            resetBoard();
            throw;
        }
        catch(invalid_argument &e){
            resetBoard();
            throw;
        }
    }
    const bool PieceKnight[13] = { false, false, true, false, false, false, false, false, true, false, false, false, false };
    const bool PieceKing[13] = { false, false, false, false, false, false, true, false, false, false, false, false, true };
    const bool PieceRookQueen[13] = { false, false, false, false, true, true, false, false, false, false, true, true, false };
    const bool PieceBishopQueen[13] = { false, false, false, true, false, true, false, false, false, true, false, true, false };

    int Board::parseMove(const string &mov){
        if(mov[1] > '8' || mov[1] < '1') throw IllegalMove(impl->hisPly, mov, "Illegal Move");
        if(mov[3] > '8' || mov[3] < '1') throw IllegalMove(impl->hisPly, mov, "Illegal Move");
        if(mov[0] > 'h' || mov[0] < 'a') throw IllegalMove(impl->hisPly, mov, "Illegal Move");
        if(mov[2] > 'h' || mov[2] < 'a') throw IllegalMove(impl->hisPly, mov, "Illegal Move");
        int from = FR2SQ(mov[0] - 'a', mov[1] - '1');
        int to = FR2SQ(mov[2] - 'a', mov[3] - '1');
        if(!impl->isSqOnBoard(from) || !impl->isSqOnBoard(to)){
            throw IllegalMove(impl->hisPly, mov, "Illegal Move");
        }
        //assert(impl->isSqOnBoard(from) && impl->isSqOnBoard(to));
        generateAllMoves();
        int m = 0;
        int promPce = EMPTY;
        for(int i = 0 ; i < impl->mlist.mCount ; ++i){
            m = impl->mlist.move[i];
            if(FROMSQ(m) == from && TOSQ(m) == to){
                promPce = PROMOTED(m);
                if(promPce != EMPTY){
                    if(IsRQ(promPce) && !IsBQ(promPce) && mov[4]=='r') {
                        return m;
                    } else if(!IsRQ(promPce) && IsBQ(promPce) && mov[4]=='b') {
                        return m;
                    } else if(IsRQ(promPce) && IsBQ(promPce) && mov[4]=='q') {
                        return m;
                    } else if(IsKn(promPce)&& mov[4]=='n') {
                        return m;
                    }
                    continue;
                }
                return m;
            }
        }
        //cout << "TEST2" << endl;
        throw IllegalMove(impl->hisPly, mov, "Illegal Move");
    }

    void Board::addWindow(MainWindow &w){
        this->w = &w;
    }

    void Board::updateView(){
        if(w != NULL){
            for(int i = 0; i < FSIZE ; i++){
                for(int j = 0; j < RSIZE ; j++){
                    if(impl->square[i][j] != NULL){
                        w->addPiece(FR2SQ(i,j), impl->square[i][j]->getType());
                    }
                }
            }
        }
    }

    void Board::resetBoard(){
        for(int i = 0 ; i < FSIZE ; i++){
            for(int j = 0 ; j < RSIZE ; j++){
                if(impl->square[i][j] != NULL) delete impl->square[i][j];
                impl->square[i][j] = NULL;
            }
        }
        for(int i = 0 ; i < 13; i++) {
            impl->pceNum[i] = 0;
        }
        impl->kingSq[WHITE] = impl->kingSq[BLACK] = NO_SQ;
        impl->side = BOTH;
        impl->enPas = NO_SQ;
        impl->fiftyMove = 0;
        impl->ply = 0;
        impl->hisPly = 0;
        impl->castlePerm = 0;
        impl->posKey = 0ULL;
    }
    void Board::addWhitePlayer(Player *p){
        impl->addWhitePlayer(p);
    }
    void Board::addBlackPlayer(Player *p) {
        impl->addBlackPlayer(p);
    }

    Player Board::getWhitePlayer()
    {
        return *(impl->white);
    }
    Player Board::getBlackPlayer()
    {
        return *(impl->black);
    }
    const int KnDir[8][2] = { {-1,-2}, {-2,-1}, {-2,1}, {-1,2}, {1,2}, {2,1}, {2,-1}, {1,-2}  };
    const int RkDir[4][2] = { {-1,0}, {0,1}, {1,0}, {0,-1} };
    const int BiDir[4][2] = { {-1,1}, {1,1}, {1,-1}, {-1,-1} };
    const int KiDir[8][2] = { {-1,0}, {0,1}, {1,0}, {0,-1}, {-1,1}, {1,1}, {1,-1}, {-1,-1} };
    bool Board::Inner::sqAttacked(const int sq, const int side){
        int pce;
        int dir[2];
        int tempsq[2];
        int color;
        // pawns
        if(side == WHITE) {
            if(SQ2F(sq)+1 >= FILE_A && SQ2F(sq)+1 <= FILE_H && SQ2R(sq)-1 >= RANK_1 && SQ2R(sq)-1 <=RANK_8){
                if(square[SQ2F(sq)+1][SQ2R(sq)-1] != NULL){
                    if(square[SQ2F(sq)+1][SQ2R(sq)-1]->getType() == wP){
                        return true;
                    }
                }
            }
            if(SQ2F(sq)-1 >= FILE_A && SQ2F(sq)-1 <= FILE_H && SQ2R(sq)-1 >= RANK_1 && SQ2R(sq)-1 <=RANK_8){
                if(square[SQ2F(sq)-1][SQ2R(sq)-1] != NULL){
                    if(square[SQ2F(sq)-1][SQ2R(sq)-1]->getType() == wP){
                        return true;
                    }
                }
            }
        }
        else{
            if(SQ2F(sq)+1 >= FILE_A && SQ2F(sq)+1 <= FILE_H && SQ2R(sq)+1 >= RANK_1 && SQ2R(sq)+1 <=RANK_8){
                if(square[SQ2F(sq)+1][SQ2R(sq)+1] != NULL){
                    if(square[SQ2F(sq)+1][SQ2R(sq)+1]->getType() == bP){
                        return true;
                    }
                }
            }
            if(SQ2F(sq)-1 >= FILE_A && SQ2F(sq)-1 <= FILE_H && SQ2R(sq)+1 >= RANK_1 && SQ2R(sq)+1 <=RANK_8){
                if(square[SQ2F(sq)-1][SQ2R(sq)+1] != NULL){
                    if(square[SQ2F(sq)-1][SQ2R(sq)+1]->getType() == bP){
                        return true;
                    }
                }
            }
        }
        // knights
        for(int i = 0 ; i < 8 ; ++i){
            if(SQ2F(sq) + KnDir[i][0] >= FILE_A && SQ2F(sq) + KnDir[i][0] <= FILE_H && SQ2R(sq) + KnDir[i][1] >= RANK_1 && SQ2R(sq) + KnDir[i][1] <= RANK_8){
                if(square[SQ2F(sq) + KnDir[i][0]][SQ2R(sq) + KnDir[i][1]] != NULL){
                    pce = square[SQ2F(sq) + KnDir[i][0]][SQ2R(sq) + KnDir[i][1]]->getType();
                    color = square[SQ2F(sq) + KnDir[i][0]][SQ2R(sq) + KnDir[i][1]]->getColor();
                    if(IsKn(pce) && color == side){
                        return true;
                    }
                }
            }
        }
        // rooks, queens
        for(int i = 0 ; i < 4; ++i){
            dir[0] = RkDir[i][0];
            dir[1] = RkDir[i][1];
            tempsq[0] = SQ2F(sq) + dir[0];
            tempsq[1] = SQ2R(sq) + dir[1];
            while(tempsq[0] >= FILE_A && tempsq[0] <= FILE_H && tempsq[1] >= RANK_1 && tempsq[1] <= RANK_8){
                if(square[tempsq[0]][tempsq[1]] != NULL){
                    pce = square[tempsq[0]][tempsq[1]]->getType();
                    color = square[tempsq[0]][tempsq[1]]->getColor();
                    if(IsRQ(pce) && color == side){
                        return true;
                    }
                    break;
                }
                tempsq[0] += dir[0];
                tempsq[1] += dir[1];
            }
        }
        // bishops, queens
        for(int i = 0; i < 4; ++i){
            dir[0] = BiDir[i][0];
            dir[1] = BiDir[i][1];
            tempsq[0] = SQ2F(sq) + dir[0];
            tempsq[1] = SQ2R(sq) + dir[1];
            while(tempsq[0] >= FILE_A && tempsq[0] <= FILE_H && tempsq[1] >= RANK_1 && tempsq[1] <= RANK_8){
                if(square[tempsq[0]][tempsq[1]] != NULL){
                    pce = square[tempsq[0]][tempsq[1]]->getType();
                    color = square[tempsq[0]][tempsq[1]]->getColor();
                    if(IsBQ(pce) && color == side){
                        return true;
                    }
                    break;
                }
                tempsq[0] += dir[0];
                tempsq[1] += dir[1];
            }
        }
        // kings
        for(int i = 0 ; i < 8 ; ++i){
            if(SQ2F(sq) + KiDir[i][0] >= FILE_A && SQ2F(sq) + KiDir[i][0] <= FILE_H && SQ2R(sq) + KiDir[i][1] >= RANK_1 && SQ2R(sq) + KiDir[i][1] <= RANK_8){
                if(square[SQ2F(sq) + KiDir[i][0]][SQ2R(sq) + KiDir[i][1]] != NULL){
                    pce = square[SQ2F(sq) + KiDir[i][0]][SQ2R(sq) + KiDir[i][1]]->getType();
                    color = square[SQ2F(sq) + KiDir[i][0]][SQ2R(sq) + KiDir[i][1]]->getColor();
                    if(IsKi(pce) && color == side){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Board::Inner::isCheck(){
        return sqAttacked(kingSq[side], (side^1));
    }
    bool Board::Inner::isCheckMate(){
        if(isCheck()){
            int m;
            generateAllMoves();
            for(int i = 0 ; i < mlist.mCount ; ++i){
                //cout << game.printMove(m) << endl;
                m = mlist.move[i];

                if(makeMove(m)){
                    takeMove();
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    bool Board::Inner::isStaleMate(){
        if(!isCheck()){
            int m;
            generateAllMoves();
            for(int i = 0 ; i < mlist.mCount ; ++i){
                //cout << game.printMove(m) << endl;
                m = mlist.move[i];

                if(makeMove(m)){
                    takeMove();
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    int Board::getSide(){
        return impl->side;
    }
    bool Board::getGameState(){
        return impl->isGame;
    }
    MoveList Board::getMoveList(){
        return impl->mlist;
    }

    void Board::saveHistory(ostream &out)
    {
        for(int i = 0 ; i < impl->history.size() ; i++){
            if(i % 2){
                out << printMove(impl->history[i].move) << endl;
            }
            else if(i != impl->history.size() - 1) out << printMove(impl->history[i].move) << " ";
        }
    }
    /* Move Generation */
    bool Board::Inner::isSqOnBoard(const int sq){
        if(sq >= A1 || sq <= NO_SQ){
            return true;
        }
        return false;
    }
    void Board::Inner::addQuietMove(const int move){
        mlist.move[mlist.mCount] = move;
        mlist.mCount++;
    }
    void Board::Inner::addCaptureMove(const int move){
        mlist.move[mlist.mCount] = move;
        mlist.mCount++;
    }
    void Board::Inner::addEnPassantMove(const int move){
        mlist.move[mlist.mCount] = move;
        mlist.mCount++;
    }
    void Board::Inner::addWhitePawnCapMove(const int from, const int to, const int cap){
        if(SQ2R(from) == RANK_7){
            addCaptureMove(MOVE(from, to, cap, wQ, 0));
            addCaptureMove(MOVE(from, to, cap, wR, 0));
            addCaptureMove(MOVE(from, to, cap, wB, 0));
            addCaptureMove(MOVE(from, to, cap, wN, 0));
        }
        else{
            addCaptureMove(MOVE(from, to, cap, EMPTY, 0));
        }
    }
    void Board::Inner::addWhitePawnMove(const int from, const int to){
        if(SQ2R(from) == RANK_7) {
            addQuietMove(MOVE(from, to, EMPTY, wQ, 0));
            addQuietMove(MOVE(from, to, EMPTY, wR, 0));
            addQuietMove(MOVE(from, to, EMPTY, wB, 0));
            addQuietMove(MOVE(from, to, EMPTY, wN, 0));
        }
        else{
            addQuietMove(MOVE(from, to, EMPTY, EMPTY, 0));
        }
    }
    void Board::Inner::addBlackPawnCapMove(const int from, const int to, const int cap){
        if(SQ2R(from) == RANK_2){
            addCaptureMove(MOVE(from, to, cap, bQ, 0));
            addCaptureMove(MOVE(from, to, cap, bR, 0));
            addCaptureMove(MOVE(from, to, cap, bB, 0));
            addCaptureMove(MOVE(from, to, cap, bN, 0));
        }
        else{
            addCaptureMove(MOVE(from, to, cap, EMPTY, 0));
        }
    }
    void Board::Inner::addBlackPawnMove(const int from, const int to){
        if(SQ2R(from) == RANK_2) {
            addQuietMove(MOVE(from, to, EMPTY, bQ, 0));
            addQuietMove(MOVE(from, to, EMPTY, bR, 0));
            addQuietMove(MOVE(from, to, EMPTY, bB, 0));
            addQuietMove(MOVE(from, to, EMPTY, bN, 0));
        }
        else{
            addQuietMove(MOVE(from, to, EMPTY, EMPTY, 0));
        }
    }
    void Board::Inner::generateWhitePawnMoves(const int sq){
        assert(isSqOnBoard(sq));
        int pce = EMPTY;
        if(square[SQ2F(sq)][SQ2R(sq) + 1] == NULL){
            addWhitePawnMove(sq, sq+8);
            if(SQ2R(sq) == RANK_2 && square[SQ2F(sq)][SQ2R(sq) + 2] == NULL){
                addQuietMove(MOVE(sq, sq+16, EMPTY, EMPTY, MFLAGPS));
            }
        }
        if(SQ2F(sq) + 1 <= FILE_H && SQ2R(sq) + 1 <= RANK_8){
            if(square[SQ2F(sq) + 1][SQ2R(sq) + 1] != NULL){
                if(square[SQ2F(sq) + 1][SQ2R(sq) + 1]->getColor() == BLACK){
                    pce = square[SQ2F(sq) + 1][SQ2R(sq) + 1]->getType();
                    addWhitePawnCapMove(sq, sq+9, pce);
                }
            }
        }
        if(SQ2F(sq) - 1 >= FILE_A && SQ2R(sq) + 1 <= RANK_8){
            if(square[SQ2F(sq) - 1][SQ2R(sq) + 1] != NULL){
                if(square[SQ2F(sq) - 1][SQ2R(sq) + 1]->getColor() == BLACK){
                    pce = square[SQ2F(sq) - 1][SQ2R(sq) + 1]->getType();
                    addWhitePawnCapMove(sq, sq+7, pce);
                }
            }
        }
        if(enPas != NO_SQ){
            if(sq + 9 == enPas && (SQ2F(sq) + 1) <= FILE_H){
                addEnPassantMove(MOVE(sq, sq+9, EMPTY, EMPTY, MFLAGEP));
            }
            if(sq + 7 == enPas){
                addEnPassantMove(MOVE(sq, sq+7, EMPTY, EMPTY, MFLAGEP));
            }
        }
    }
    void Board::Inner::generateBlackPawnMoves(const int sq){
        assert(isSqOnBoard(sq));
        int pce = EMPTY;
        if(square[SQ2F(sq)][SQ2R(sq) - 1] == NULL){
            addBlackPawnMove(sq, sq-8);
            if(SQ2R(sq) == RANK_7 && square[SQ2F(sq)][SQ2R(sq) - 2] == NULL){
                addQuietMove(MOVE(sq, sq-16, EMPTY, EMPTY, MFLAGPS));
            }
        }
        if(SQ2F(sq) - 1 >= FILE_A && SQ2R(sq) - 1 >= RANK_1){
            if(square[SQ2F(sq) - 1][SQ2R(sq) - 1] != NULL){
                if(square[SQ2F(sq) - 1][SQ2R(sq) - 1]->getColor() == WHITE){
                    pce = square[SQ2F(sq) - 1][SQ2R(sq) - 1]->getType();
                    addBlackPawnCapMove(sq, sq-9, pce);
                }
            }
        }
        if(SQ2F(sq) + 1 <= FILE_H && SQ2R(sq) - 1 >= RANK_1){
            if(square[SQ2F(sq) + 1][SQ2R(sq) - 1] != NULL){
                if(square[SQ2F(sq) + 1][SQ2R(sq) - 1]->getColor() == WHITE){
                    pce = square[SQ2F(sq) + 1][SQ2R(sq) - 1]->getType();
                    addBlackPawnCapMove(sq, sq-7, pce);
                }
            }
        }
        if(enPas != NO_SQ){
            if(sq - 9 == enPas && (SQ2F(sq) - 1) > FILE_A) {
                addEnPassantMove(MOVE(sq, sq-9, EMPTY, EMPTY, MFLAGEP));
            }
            if(sq - 7 == enPas) {
                addEnPassantMove(MOVE(sq, sq-7, EMPTY, EMPTY, MFLAGEP));
            }
        }
    }
    void Board::Inner::generateCastling(){
        if(side == WHITE){
            /* White Castle */
            if(castlePerm & WKCA) {
                if(square[SQ2F(F1)][SQ2R(F1)] == NULL && square[SQ2F(G1)][SQ2R(G1)] == NULL){
                    if(!sqAttacked(E1, BLACK) && !sqAttacked(F1, BLACK)){
                        addQuietMove(MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA));
                    }
                }
            }
            if(castlePerm & WQCA) {
                if(square[SQ2F(D1)][SQ2R(D1)] == NULL && square[SQ2F(C1)][SQ2R(C1)] == NULL && square[SQ2F(B1)][SQ2R(B1)] == NULL){
                    if(!sqAttacked(E1, BLACK) && !sqAttacked(D1, BLACK)){
                        addQuietMove(MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA));
                    }
                }
            }
        }
        else{
            /* Black Castle */
            if(castlePerm & BKCA) {
                if(square[SQ2F(F8)][SQ2R(F8)] == NULL && square[SQ2F(G8)][SQ2R(G8)] == NULL){
                    if(!sqAttacked(E8, WHITE) && !sqAttacked(F8, WHITE)){
                        addQuietMove(MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA));
                    }
                }
            }
            if(castlePerm & BQCA) {
                if(square[SQ2F(D8)][SQ2R(D8)] == NULL && square[SQ2F(C8)][SQ2R(C8)] == NULL && square[SQ2F(B8)][SQ2R(B8)] == NULL){
                    if(!sqAttacked(E8, WHITE) && !sqAttacked(D8, WHITE)){
                        addQuietMove(MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA));
                    }
                }
            }
        }
    }
    int LoopSlidePce[8] = { wB, wR, wQ, 0, bB, bR, bQ, 0 };
    int LoopNonSlidePce[6] = { wN, wK, 0, bN, bK, 0 };
    int LoopSlideIndex[2] = { 0, 4 };
    int LoopNonSlideIndex[2] = { 0, 3 };
    int PceDir[13][8][2] = {
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,-2}, {-2,-1}, {-2,1}, {-1,2}, {1,2}, {2,1}, {2,-1}, {1,-2} },
	{ {-1,1}, {1,1}, {1,-1}, {-1,-1}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {-1,1}, {1,1}, {1,-1}, {-1,-1} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {-1,1}, {1,1}, {1,-1}, {-1,-1} },
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,-2}, {-2,-1}, {-2,1}, {-1,2}, {1,2}, {2,1}, {2,-1}, {1,-2} },
	{ {-1,1}, {1,1}, {1,-1}, {-1,-1}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {0,0}, {0,0}, {0,0}, {0,0} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {-1,1}, {1,1}, {1,-1}, {-1,-1} },
	{ {-1,0}, {0,1}, {1,0}, {0,-1}, {-1,1}, {1,1}, {1,-1}, {-1,-1} },
	};
	int NumDir[13] = { 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8 };
    void Board::Inner::generateSlidingPieceMoves(const int sq){
        assert(isSqOnBoard(sq));
        int pce;
        int dir[2];
        int tempsq[2];
        int color;
        int cap;
        pce = square[SQ2F(sq)][SQ2R(sq)]->getType();
        for(int i = 0 ; i < NumDir[pce]; i++){
            dir[0] = PceDir[pce][i][0];
            dir[1] = PceDir[pce][i][1];
            tempsq[0] = SQ2F(sq) + dir[0];
            tempsq[1] = SQ2R(sq) + dir[1];
            while(tempsq[0] >= FILE_A && tempsq[0] <= FILE_H && tempsq[1] >= RANK_1 && tempsq[1] <= RANK_8){
                // BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
                if(square[tempsq[0]][tempsq[1]] != NULL){
                    color = square[tempsq[0]][tempsq[1]]->getColor();
                    if(color == (side ^ 1) ){
                        cap = square[tempsq[0]][tempsq[1]]->getType();
                        addCaptureMove(MOVE(sq, FR2SQ(tempsq[0], tempsq[1]), cap, EMPTY, 0));
                    }
                    break;
                }
            addQuietMove(MOVE(sq, FR2SQ(tempsq[0], tempsq[1]), EMPTY, EMPTY, 0));
            tempsq[0] += dir[0];
            tempsq[1] += dir[1];
            }
        }
    }
	void Board::Inner::generateNonSlidingPieceMoves(const int sq){
	    assert(isSqOnBoard(sq));
        int pce;
        int dir[2];
        int tempsq[2];
        int color;
        int cap;
        pce = square[SQ2F(sq)][SQ2R(sq)]->getType();
        for(int i = 0 ; i < NumDir[pce]; i++){
            dir[0] = PceDir[pce][i][0];
            dir[1] = PceDir[pce][i][1];
            tempsq[0] = SQ2F(sq) + dir[0];
            tempsq[1] = SQ2R(sq) + dir[1];
            if(tempsq[0] < FILE_A || tempsq[0] > FILE_H || tempsq[1] < RANK_1 || tempsq[1] > RANK_8){
                continue;
            }
            // BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
            if(square[tempsq[0]][tempsq[1]] != NULL){
                color = square[tempsq[0]][tempsq[1]]->getColor();
                if( color == (side ^ 1) ){
                    cap = square[tempsq[0]][tempsq[1]]->getType();
                    addCaptureMove(MOVE(sq, FR2SQ(tempsq[0], tempsq[1]), cap, EMPTY, 0));
                }
                continue;
            }
            addQuietMove(MOVE(sq, FR2SQ(tempsq[0], tempsq[1]), EMPTY, EMPTY, 0));
        }
    }
    void Board::Inner::generateAllMoves(){
        mlist.mCount = 0;
        int pce = EMPTY;
        int sq = 0;
        int pceIndex = 0;
        if(side == WHITE){
            for(int i = 0 ; i < pceNum[wP] ; i++){
                sq = pList[wP][i];
                generateWhitePawnMoves(sq);
            }
        }
        else{
            for(int i = 0 ; i < pceNum[bP] ; i++){
                sq = pList[bP][i];
                generateBlackPawnMoves(sq);
            }
        }
        /* Loop for slide pieces */
        pceIndex = LoopSlideIndex[side];
        pce = LoopSlidePce[pceIndex++];
        while(pce != 0){
            for(int i = 0 ; i < pceNum[pce]; i++){
                sq = pList[pce][i];
                generateSlidingPieceMoves(sq);
            }
            pce = LoopSlidePce[pceIndex++];
        }
        /* Loop for non slide */
        pceIndex = LoopNonSlideIndex[side];
        pce = LoopNonSlidePce[pceIndex++];
        while( pce != 0) {
            for(int i = 0 ; i < pceNum[pce]; i++){
                sq = pList[pce][i];
                generateNonSlidingPieceMoves(sq);
            }
            pce = LoopNonSlidePce[pceIndex++];
        }
        /* Castling */
        generateCastling();
    }

    void Board::generateAllMoves(){
        impl->generateAllMoves();
    }
    /* End of Move Generation */
    const int CastlePerm[BSIZE] = {
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
         7, 15, 15, 15,  3, 15, 15, 11
    };
    void Board::Inner::clearPiece(const int sq){
        assert(isSqOnBoard(sq));
        int tempPceNum = -1;
        int pce = square[SQ2F(sq)][SQ2R(sq)]->getType();
        HASH_PCE(pce, sq);
        //delete square[SQ2F(sq)][SQ2R(sq)];
        square[SQ2F(sq)][SQ2R(sq)] = NULL;
        for(int i = 0 ; i < pceNum[pce] ; ++i){
            if(pList[pce][i] == sq){
                tempPceNum = i;
                break;
            }
        }
        assert(tempPceNum != -1);
        pceNum[pce]--;
        pList[pce][tempPceNum] = pList[pce][pceNum[pce]];
    }
    void Board::Inner::addPiece(const int sq, const int pce){
        assert(isSqOnBoard(sq));
        int col = pce/7;
        HASH_PCE(pce, sq);
        square[SQ2F(sq)][SQ2R(sq)] = new Piece(col, pce);
        pList[pce][pceNum[pce]] = sq;
        pceNum[pce]++;
    }
    void Board::Inner::movePiece(const int from, const int to){
        assert(isSqOnBoard(from));
        assert(isSqOnBoard(to));
        int pce = square[SQ2F(from)][SQ2R(from)]->getType();
        HASH_PCE(pce, from);
        square[SQ2F(to)][SQ2R(to)] = square[SQ2F(from)][SQ2R(from)];
        square[SQ2F(from)][SQ2R(from)] = NULL;
        HASH_PCE(pce, to);
        for(int i = 0 ; i < pceNum[pce] ; ++i){
            if(pList[pce][i] == from){
                pList[pce][i] = to;
                break;
            }
        }
    }
    bool Board::Inner::makeMove(const int move){
        //cout << *white << endl;
        Undo temp;

        int from = FROMSQ(move);
        int to = TOSQ(move);
        int side = this->side;

        assert(isSqOnBoard(from));
        assert(isSqOnBoard(to));
        temp.posKey = this->posKey;

        if(history.size() != hisPly){
            history.erase(history.begin() + hisPly, history.end());
        }

        if(move & MFLAGEP){
            if(side == WHITE){
                graveyard.insert(square[SQ2F(to-8)][SQ2R(to-8)]);
                clearPiece(to-8);
            }
            else{
                graveyard.insert(square[SQ2F(to+8)][SQ2R(to+8)]);
                clearPiece(to+8);
            }
        }
        else if(move & MFLAGCA){
            switch(to){
                case C1:
                    movePiece(A1, D1);
                    break;
                case C8:
                    movePiece(A8, D8);
                    break;
                case G1:
                    movePiece(H1, F1);
                    break;
                case G8:
                    movePiece(H8, F8);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;

        temp.move = move;
        temp.fiftyMove = this->fiftyMove;
        temp.enPas = this->enPas;
        temp.castlePerm = this->castlePerm;

        history.push_back(temp);

        castlePerm &= CastlePerm[from];
        castlePerm &= CastlePerm[to];
        enPas = NO_SQ;
        HASH_CA;
        int cap = CAPTURED(move);
        fiftyMove++;
        if(cap != EMPTY){
            graveyard.insert(square[SQ2F(to)][SQ2R(to)]);
            clearPiece(to);
            fiftyMove = 0;
        }
        hisPly++;
        ply++;
        int pce = square[SQ2F(from)][SQ2R(from)]->getType();
        if(pce == wP || pce == bP){
            fiftyMove = 0;
            if(move & MFLAGPS){
                if(side == WHITE){
                    enPas = from+8;
                    assert(SQ2R(from+8) == RANK_3);
                }
                else{
                    enPas = from-8;
                    assert(SQ2R(from-8) == RANK_6);
                }
                HASH_EP;
            }
        }
        movePiece(from, to);
        int prPce = PROMOTED(move);
        if(prPce != EMPTY){
            clearPiece(to);
            addPiece(to, prPce);
        }
        if(pce == wK || pce == bK){
            kingSq[side] = to;
        }
        this->side ^= 1;
        HASH_SIDE;
        if(sqAttacked(kingSq[side], this->side)){
            takeMove();
            return false;
        }
        return true;
    }

    bool Board::Inner::makeHistoryMove(){
        clog << "makeMoveHistory is not implemented in this Board class" << endl;
        return false;
    }

    void Board::Inner::takeMove(){
        if(history.size() == 0){
            return;
        }
        hisPly--;
        ply--;
        int move = history[hisPly].move;
        int from = FROMSQ(move);
        int to = TOSQ(move);
        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;
        castlePerm = history[hisPly].castlePerm;
        fiftyMove = history[hisPly].fiftyMove;
        enPas = history[hisPly].enPas;

        history.pop_back();

        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;
        side ^= 1;
        HASH_SIDE;
        if(MFLAGEP & move){
            if(side == WHITE){
                addPiece(to-8, bP);
                FindPiece callback(bP);
                set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
                delete *it;
                graveyard.erase(it);
            }
            else{
                addPiece(to+8, wP);
                FindPiece callback(wP);
                set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
                delete *it;
                graveyard.erase(it);
            }
        }
        else if(MFLAGCA & move){
            switch(to){
                case C1:
                    movePiece(D1, A1);
                    break;
                case C8:
                    movePiece(D8, A8);
                    break;
                case G1:
                    movePiece(F1, H1);
                    break;
                case G8:
                    movePiece(F8, H8);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        movePiece(to, from);
        int pce = square[SQ2F(from)][SQ2R(from)]->getType();
        if(pce == wK || pce == bK){
            kingSq[side] = from;
        }
        int cap = CAPTURED(move);
        if(cap != EMPTY){
            addPiece(to, cap);
            FindPiece callback(cap);
            set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
            delete *it;
            graveyard.erase(it);
        }
        if(PROMOTED(move) != EMPTY){
            int col = PROMOTED(move) / 7;
            clearPiece(from);
            addPiece(from, col == WHITE ? wP : bP);
        }
    }


    bool Board::makeMove(const int move){
        int from, to;

        if(impl->makeMove(move)){
            if(w != NULL){
                from = FROMSQ(move);
                to = TOSQ(move);
                if(move & MFLAGEP){
                    if(impl->side == WHITE){
                        w->clearPiece(to+8);
                    }
                    else{
                        w->clearPiece(to-8);
                    }
                }
                else if(move & MFLAGCA){
                    switch(to){
                        case C1:
                            w->movePiece(A1, D1);
                            break;
                        case C8:
                            w->movePiece(A8, D8);
                            break;
                        case G1:
                            w->movePiece(H1, F1);
                            break;
                        case G8:
                            w->movePiece(H8, F8);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
                int prPce = PROMOTED(move);
                    if(prPce != EMPTY){
                        w->clearPiece(from);
                        w->addPiece(from, prPce);
                    }

                w->movePiece(from, to);
                w->unSet();
                w->addMove(printMove(move), impl->hisPly - 1);
                w->setSquare(from);
                w->setSquare(to);
                if(impl->isCheck()){
                    w->setCheck(impl->kingSq[impl->side]);
                }

                if(impl->isCheckMate()){
                    w->setCheckMate( impl->side ^ 1 );
                }
                if(impl->isStaleMate()){
                    w->setStaleMate();
                }

            }
            return true;
        }
        return false;
    }
    bool Board::makeHistoryMove(){
        if(impl->makeHistoryMove()){
            if(w != NULL){
                int move = impl->history[impl->hisPly - 1].move;

                int from = FROMSQ(move);
                int to = TOSQ(move);
                if(move & MFLAGEP){
                    if(impl->side == WHITE){
                        w->clearPiece(to+8);
                    }
                    else{
                        w->clearPiece(to-8);
                    }
                }
                else if(move & MFLAGCA){
                    switch(to){
                        case C1:
                            w->movePiece(A1, D1);
                            break;
                        case C8:
                            w->movePiece(A8, D8);
                            break;
                        case G1:
                            w->movePiece(H1, F1);
                            break;
                        case G8:
                            w->movePiece(H8, F8);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
                int prPce = PROMOTED(move);
                if(prPce != EMPTY){
                    w->clearPiece(from);
                    w->addPiece(from, prPce);
                }
                w->movePiece(from, to);
                w->unSet();
                //w->addMove(printMove(move), impl->hisPly - 1);
                if(impl->isCheck()){
                    w->setCheck(impl->kingSq[impl->side]);
                }
            }
           return true;
        }
        return false;
    }
    void Board::takeMove(){
        int from, to;
        if(w != NULL){
            if(impl->hisPly != 0){
                int move = impl->history[impl->hisPly - 1].move;
                from = FROMSQ(move);
                to = TOSQ(move);
                if(MFLAGEP & move){
                    if(impl->side == WHITE){
                        w->addPiece(to-8, bP);
                    }
                    else{
                        w->addPiece(to+8, wP);
                    }
                }
                else if(MFLAGCA & move){
                    switch(to){
                        case C1:
                            w->movePiece(D1, A1);
                            break;
                        case C8:
                            w->movePiece(D8, A8);
                            break;
                        case G1:
                            w->movePiece(F1, H1);
                            break;
                        case G8:
                            w->movePiece(F8, H8);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
                w->movePiece(to, from);
                int cap = CAPTURED(move);
                if(cap != EMPTY){
                    w->addPiece(to, cap);
                }
                if(PROMOTED(move) != EMPTY){
                    int col = PROMOTED(move) / 7;
                    w->clearPiece(from);
                    w->addPiece(from, col == WHITE ? wP : bP);
                }
                if(!keepHistory){
                    w->clearMove(impl->hisPly - 1);
                }
                w->unSet();
            }
            else return;
        }
        impl->takeMove();
        if(impl->isCheck()){
            w->setCheck(impl->kingSq[impl->side]);
        }
    }

    bool Board::operator==(const Board &b){
        if(impl->posKey != b.impl->posKey){
            return false;
        }
        return true;
    }

    Board::PceIterator Board::findPiece(int PieceType){
        FindPiece callback(PieceType);

        return find_if(_begin(), _end(), callback);
    }

    void Board::Inner::cleanUp(){
        for(int i = 0 ; i < FSIZE ; i++){
            for(int j = 0 ; j < RSIZE ; j++){
                if(square[i][j] != NULL){
                    delete square[i][j];
                    square[i][j] = NULL;
                }
            }
        }
        set<Piece*>::iterator it;
        for(it = graveyard.begin() ; it != graveyard.end() ; ++it){
            delete *it;
        }
        if(white != NULL){
            delete white;
        }
        if(black != NULL){
            delete black;
        }
    }
    string Board::printBitBoard(U64 bb){
        stringstream ss;
        U64 shiftMe = 1ULL;
        int sq = 0;
        for(int i = RANK_8 ; i >= RANK_1 ; --i){
            for(int j = FILE_A ; j <= FILE_H ; ++j){
                sq = FR2SQ(j, i);
                if((shiftMe << sq) & bb){
                    ss << "X ";
                }
                else{
                    ss << "0 ";
                }
            }
            ss << endl;
        }
        return ss.str();
    }
    std::string Board::printSq(const int sq){
        stringstream ss;
        int f = SQ2F(sq);
        int r = SQ2R(sq);
        ss << (char)('a'+f) << (char)('1'+r);
        return ss.str();
    }
    std::string Board::printMove(const int move){
        stringstream ss;
        int ff = SQ2F(FROMSQ(move));
        int rf = SQ2R(FROMSQ(move));
        int ft = SQ2F(TOSQ(move));
        int rt = SQ2R(TOSQ(move));
        int promoted = PROMOTED(move);
        if(promoted){
            char pchar = 'q';
            if(IsKn(promoted)){
                pchar = 'n';
            }
            else if(IsRQ(promoted) && !IsBQ(promoted)){
                pchar = 'r';
            }
            else if(!IsRQ(promoted) && IsBQ(promoted)){
                pchar = 'b';
            }
            ss << (char)('a'+ff) << (char)('1'+rf) << (char)('a'+ft) << (char)('1'+rt) << pchar;
        }
        else{
            ss << (char)('a'+ff) << (char)('1'+rf) << (char)('a'+ft) << (char)('1'+rt);
        }
        return ss.str();
    }
    string Board::printMoveList(){
        MoveList mlist = getMoveList();
        stringstream ss;
        int move = 0;
        ss << "MoveList:" << endl;
        for(int i = 0; i < mlist.mCount; ++i) {
            move = mlist.move[i];
            ss << "Move: " << i+1 << " " << printMove(move) << endl;
        }
        ss << "MoveList Total " << mlist.mCount << " Moves" << endl;
        return ss.str();
    }
    string Board::printGraveyard(){
        stringstream ss;
        set<Piece*>::iterator it;
        for(it = impl->graveyard.begin() ; it != impl->graveyard.end() ; it++){
            ss << **it << " ";
        }
        return ss.str();
    }
    template <typename T>
    T Board::showBoard() const {
        char pceChar[] = ".PNBRQKpnbrqk";
        char sideChar[] = "wb-";
        stringstream ss;
        int piece;
        ss << "Game Board:" << endl;
        for(int r = RANK_8; r >= RANK_1; r--) {
            ss << r+1 << "   ";
            for(int f = FILE_A; f <= FILE_H; f++) {
                if(impl->square[f][r] != NULL) piece = impl->square[f][r]->getType();
                else piece = EMPTY;
                ss << setw(3) << pceChar[piece];
            }
            ss << endl;
        }
        ss << endl;
        ss << "    ";
        for(int f = FILE_A; f <= FILE_H; f++){
            ss << setw(3) << (char)('A'+f);
        }
        ss << endl;
        ss << "side:" << sideChar[impl->side] << endl;
        ss << "enPas:" << impl->enPas << endl;
        ss << "castle:";
        ss << (char)((impl->castlePerm & WKCA) ? 'K' : '-');
        ss << (char)((impl->castlePerm & WQCA) ? 'Q' : '-');
        ss << (char)((impl->castlePerm & BKCA) ? 'k' : '-');
        ss << (char)((impl->castlePerm & BQCA) ? 'q' : '-');
        ss << endl;

        ss << "posKey: " << hex << impl->posKey << endl;
        if(impl->isCheck()){
            if(impl->isCheckMate()){
                ss << "Check Mate!" << endl;
            }
            else{
                ss << "Check!" << endl;
            }
        }
        set<Piece*>::iterator it;
        ss << "Taken Pieces:" << endl;
        for(it = impl->graveyard.begin() ; it != impl->graveyard.end() ; ++it){
            ss << **it << " ";
        }
        return ss.str();
    }

    class ReplayBoard::ReplayInner : public Board::Inner{
        protected:
            friend class ReplayBoard;
            void takeMove();
            bool makeHistoryMove();
            //void loadHistory(const Inner &i);
        public:
    };
    void ReplayBoard::ReplayInner::takeMove(){
        if(hisPly == 0){
            return;
        }
        hisPly--;
        ply--;
        int move = history[hisPly].move;
        int from = FROMSQ(move);
        int to = TOSQ(move);
        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;
        castlePerm = history[hisPly].castlePerm;
        fiftyMove = history[hisPly].fiftyMove;
        enPas = history[hisPly].enPas;

        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;
        side ^= 1;
        HASH_SIDE;
        if(MFLAGEP & move){
            if(side == WHITE){
                addPiece(to-8, bP);
                FindPiece callback(bP);
                set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
                delete *it;
                graveyard.erase(it);
            }
            else{
                addPiece(to+8, wP);
                FindPiece callback(wP);
                set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
                delete *it;
                graveyard.erase(it);
            }
        }
        else if(MFLAGCA & move){
            switch(to){
                case C1:
                    movePiece(D1, A1);
                    break;
                case C8:
                    movePiece(D8, A8);
                    break;
                case G1:
                    movePiece(F1, H1);
                    break;
                case G8:
                    movePiece(F8, H8);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        movePiece(to, from);
        int pce = square[SQ2F(from)][SQ2R(from)]->getType();
        if(pce == wK || pce == bK){
            kingSq[side] = from;
        }
        int cap = CAPTURED(move);
        if(cap != EMPTY){
            addPiece(to, cap);
            FindPiece callback(cap);
            set<Piece*>::iterator it = find_if(graveyard.begin(), graveyard.end(), callback);
            delete *it;
            graveyard.erase(it);
        }
        if(PROMOTED(move) != EMPTY){
            int col = PROMOTED(move) / 7;
            clearPiece(from);
            addPiece(from, col == WHITE ? wP : bP);
        }
    }

    bool ReplayBoard::ReplayInner::makeHistoryMove(){
        if(hisPly == history.size()){
            return false;
        }
        int move = history[hisPly].move;

        int from = FROMSQ(move);
        int to = TOSQ(move);
        int side = this->side;

        assert(isSqOnBoard(from));
        assert(isSqOnBoard(to));

        if(move & MFLAGEP){
            if(side == WHITE){
                graveyard.insert(square[SQ2F(to-8)][SQ2R(to-8)]);
                clearPiece(to-8);
            }
            else{
                graveyard.insert(square[SQ2F(to+8)][SQ2R(to+8)]);
                clearPiece(to+8);
            }
        }
        else if(move & MFLAGCA){
            switch(to){
                case C1:
                    movePiece(A1, D1);
                    break;
                case C8:
                    movePiece(A8, D8);
                    break;
                case G1:
                    movePiece(H1, F1);
                    break;
                case G8:
                    movePiece(H8, F8);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        if(enPas != NO_SQ){
            HASH_EP;
        }
        HASH_CA;

        castlePerm &= CastlePerm[from];
        castlePerm &= CastlePerm[to];
        enPas = NO_SQ;
        HASH_CA;
        int cap = CAPTURED(move);
        fiftyMove++;
        if(cap != EMPTY){
            graveyard.insert(square[SQ2F(to)][SQ2R(to)]);
            clearPiece(to);
            fiftyMove = 0;
        }
        hisPly++;
        ply++;
        int pce = square[SQ2F(from)][SQ2R(from)]->getType();
        if(pce == wP || pce == bP){
            fiftyMove = 0;
            if(move & MFLAGPS){
                if(side == WHITE){
                    enPas = from+8;
                    assert(SQ2R(from+8) == RANK_3);
                }
                else{
                    enPas = from-8;
                    assert(SQ2R(from-8) == RANK_6);
                }
                HASH_EP;
            }
        }
        movePiece(from, to);
        int prPce = PROMOTED(move);
        if(prPce != EMPTY){
            clearPiece(to);
            addPiece(to, prPce);
        }
        if(pce == wK || pce == bK){
            kingSq[side] = to;
        }
        this->side ^= 1;
        HASH_SIDE;
        if(sqAttacked(kingSq[side], this->side)){
            takeMove();
            return false;
        }
        return true;
    }

    ReplayBoard::ReplayBoard(){
        delete impl;
        impl = new ReplayInner;
        resetBoard();
        keepHistory = true;
    }

    ReplayBoard::ReplayBoard(const Board &b)
        :ReplayBoard()
    {
        #ifdef DEBUG
            clog << "ReplayBoard copy constructor called!" << endl;
        #endif // DEBUG
    }


    int ReplayBoard::parseGame(istream &in){
        string input;
        while(!in.eof()){
            int move;
            try{
                in >> input;
                move = parseMove(&input[0]);
                if(move == NOMOVE){
                    //cout << "TEST1" << endl;
                    throw IllegalMove(impl->hisPly, input, "Illegal Move");
                }

                if(impl->makeMove(move)){
                    if(w != NULL){
                        w->addMove(printMove(move), impl->hisPly - 1);
                    }
                }
            }
            catch(IllegalMove &e){
                for(int i = 0 ; i < impl->history.size() ; i++){
                    impl->takeMove();
                }
                throw;
            }
        }
        for(int i = 0 ; i < impl->history.size() ; i++){
            impl->takeMove();
        }

        return 0;
    }

    void ReplayBoard::otherImpl(){

        delete impl;
        impl = new ReplayInner();

    }

    template string Board::showBoard() const;
    ostream& operator<<(ostream& o, Board &game){
        o << game.showBoard<string>();
        return o;
    }
}
