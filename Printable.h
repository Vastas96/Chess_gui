#ifndef PRINTABLE_H
#define PRINTABLE_H
#include <string>
#include <iostream>

#define ostream std::ostream

namespace Chess{
    class Printable
    {
        public:
            Printable();
            virtual ~Printable();
            friend ostream& operator<<(ostream& o, const Printable &obj);
            virtual std::string toString() const = 0;
        private:
    };
    ostream& operator<<(ostream& o, const Printable &obj);
}
#endif // PRINTABLE_H
