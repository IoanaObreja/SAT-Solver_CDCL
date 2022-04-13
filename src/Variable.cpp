#include "Variable.h"

Variable::Variable()
{
    level = 0;
    antecedent = 0;
    value = 0; // 0 unassigned, -1 false, 1 true
}

Variable::Variable(int v, int l, int a)
{
    value = v;
    level = l;
    antecedent = a;
}

Variable::~Variable()
{
    //dtor
}
