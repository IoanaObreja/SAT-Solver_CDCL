#include <fstream>
#include "Formula.h"
#include "Clause.h"

Formula::Formula()
{
    //ctor
}

Formula::Formula(int v, int c)
{
    variables = v;
    nrclauses = c;
    for(int i=1;i<=variables;i++) {
        this->var_app[i] = 0;
        this->var_app[(-1)*i] = 0;
    }
}

Formula::~Formula()
{
    //dtor
}
