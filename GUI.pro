#-------------------------------------------------
#
# Project created by QtCreator 2016-12-21T22:30:09
#
#-------------------------------------------------

QT       += core gui \
            multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app

CONFIG += console\
    c++11
QT += widgets

SOURCES += main.cpp\
        mainwindow.cpp \
    Board.cpp \
    Piece.cpp \
    Player.cpp \
    Printable.cpp \
    qpiece.cpp \
    qplayer.cpp \
    mytcpsocket.cpp

HEADERS  += mainwindow.h \
    Board.h \
    defs.h \
    myExceptions.h \
    MySort.h \
    Piece.h \
    Player.h \
    Printable.h \
    qpiece.h \
    FindPiece.h \
    qplayer.h \
    ReplayBoard.h \
    mytcpsocket.h

LIBS += -lwsock32

DEFINES += DEBUG

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

DISTFILES +=
