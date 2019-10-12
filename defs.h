#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <cstdint>
#include <cstring>

#define U64 uint64_t
#define BSIZE 64
#define RSIZE 8
#define FSIZE 8
#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define NOMOVE 0

enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};

enum {WHITE, BLACK, BOTH};

enum {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};
//Castling rights in 4 bits
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

struct MoveList{
    int move[MAXPOSITIONMOVES];
    int mCount;
};

struct Undo{
    int move;
    int castlePerm;
    int enPas;
    int fiftyMove;
    U64 posKey;
};

/* Networking Structs */

enum PacketTypes {

    INIT_CONNECTION = 0,

    MAKE_MOVE = 1,

    NEW_GAME = 2,

    JOIN_GAME = 3,

    SUCCESS = 4,

    FAILURE = 5
};

struct Packet {

    unsigned int packet_type;

    void serialize(char * data) {
        memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(Packet));
    }
};

/* End of Networking structs */

/* GAME MOVE */

/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

/* FLAGS */

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

/* MACROS */

#define FR2SQ(f,r) ( (f) +  (r) * 8  )

#define SQ2R(sq) ( (sq) / 8 )
#define SQ2F(sq) ( (sq) % 8 )

#define SQ2X(sq) ( ((sq) % 8) * 60 )
#define SQ2Y(sq) ( (420) - (((sq) / 8) * 60) )

#define SQ2XF(sq) ( (420) - ((sq) % 8) * 60 )
#define SQ2YF(sq) ( (((sq) / 8) * 60) )

#define CLRBIT(bb,sq) ((bb) &= clearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= setMask[(sq)])

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))

#define HASH_PCE(pce,sq) (posKey ^= (pieceKeys[(pce)][(sq)]))
#define HASH_CA (posKey ^= (castleKeys[(castlePerm)]))
#define HASH_SIDE (posKey ^= (sideKey))
#define HASH_EP (posKey ^= (pieceKeys[EMPTY][(enPas)]))

#define RAND_64  ((U64)rand() | \
                 ((U64)rand() << 15) |  \
                 ((U64)rand() << 30) |  \
                 ((U64)rand() << 45) | \
                 (((U64)rand() & 0xf) << 60))

#endif // DEFS_H_INCLUDED
