#ifndef FORMULA_H
#define FORMULA_H
#include "Clause.h"
#include <map>
using namespace std;
class Formula
{
    public:
        Formula();
        Formula(int, int);
        virtual ~Formula();
        list<Clause> clauses;
        map<int, float> var_app;
        int nrclauses, variables;
    protected:
    private:
};

#endif // FORMULA_H
