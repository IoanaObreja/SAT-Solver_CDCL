#include "Clause.h"
#include <list>
Clause::Clause()
{
    flag = 0;
    nr_literals = 0;
    nr_literals_false = 0;
}


Clause::~Clause()
{
    //dtor
}
