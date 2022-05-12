#ifndef CLAUSE_H
#define CLAUSE_H
#include <list>

using namespace std;

struct literal {
    int name;
    int flag = 0;  /// 0 when unassigned, level when true, -level when false
    void operator = (literal l) {
        name = l.name;
        flag = l.flag;
    }
    bool operator == (literal l) {
        return name == l.name && flag == l.flag;
    }
    bool operator < (literal l) {
        return name < l.name;
    }
    bool operator > (literal l) {
        return name > l.name;
    }

};
class Clause
{
    public:
        Clause();
        virtual ~Clause();
        int index;
        int flag; /// 0 when unassigned, level when sat
        int nr_literals;
        int nr_literals_false;
        list<literal> lst;
        void operator = (Clause cls){
            index = cls.index;
            flag = cls.flag;
            nr_literals = cls.nr_literals;
            nr_literals_false = cls.nr_literals_false;
            lst = cls.lst;
        }
    protected:
    private:
};

#endif // CLAUSE_H
