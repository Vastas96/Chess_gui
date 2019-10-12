#include "Printable.h"

namespace Chess{

    using namespace std;

    Printable::Printable()
    {
        //ctor
    }

    Printable::~Printable()
    {
        //dtor
    }

    ostream& operator<<(ostream& o, const Printable &obj){
        o << obj.toString();
        return o;
    }
}
