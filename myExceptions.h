#ifndef MYEXCEPTIONS_H_INCLUDED
#define MYEXCEPTIONS_H_INCLUDED
#include <exception>
#include <string>
#include <stdexcept>
#include <iostream>


class BoardException{
    protected:
        int hisPly;
        std::string input;
    public:
        BoardException(){ };
        BoardException(const char *input) : input(input){
            this->input.erase(this->input.begin()+1, this->input.end());
        }
        BoardException(const int hisPly, std::string input) : hisPly(hisPly), input(input){ }
        int getHisPly(){
            return hisPly;
        }
        std::string getInput(){
            return input;
        }
};

class IllegalMove : public std::exception, public BoardException{
    public:
        IllegalMove(){
            this->message = "Unknown Illegal Move";
        }
        IllegalMove(const int hisPly, const std::string input, const char *msg) : BoardException(hisPly, input){
            this->message = msg;
        }
        virtual const char* what() const throw() {  return message; };
    protected:
        const char* message;
};

class InputException : public std::exception{
    public:
        InputException(const char* message){
            this->message = message;
        }
        InputException(){
            this->message = "Default Input Exception";
        }
        virtual const char* what() const throw() {  return message; };
    protected:
        const char* message;

};

class EmptyFen : public InputException{
    public:
        EmptyFen(){
            this->message = "Fen is Empty";
        }
};

class FenException : public std::exception, public BoardException{
    public:
        FenException(){
            this->message = "Unknown Fen Exception";
        }
        FenException(const char *input, const char *msg) : BoardException(input){
            this->message = msg;
        }
        virtual const char* what() const throw() {  return message; };
    protected:
        const char* message;
};

/* BOARD EXCEPTIONS */
// At which move exception happened

#endif // MYEXCEPTIONS_H_INCLUDED
