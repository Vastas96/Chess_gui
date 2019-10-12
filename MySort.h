#ifndef MYSORT_H
#define MYSORT_H

template<typename T>
class MySort
{
    public:
        bool operator()(const T &a, const T &b) const{
            return a < b;
        }
    protected:
    private:
};

#endif // MYSORT_H
