#include <limits>
#include <fstream>
#include <list>
#include "Clause.h"
#include <map>
#include <cstdlib>
#include "Formula.h"
#include "Variable.h"
#include <cmath>

using namespace std;
ofstream gg ("output.out");

int pickFirstFound(Formula formula) {
    /// returns first literal found

    for(auto& cls: formula.clauses)
        if(cls.flag == 0)
            for(auto& lit: cls.lst)
                if(lit.flag == 0)
                    return lit.name;
}

int pickMostPopular(Formula formula) {

    /// returns variable with most occurrences
    int maxi = -1, var;
    for(int i=1;i<=formula.variables;i++) {
        if(formula.var_app[i] > maxi) {
            maxi = formula.var_app[i];
            var = i;
        }
        if(formula.var_app[(-1)*i] > maxi) {
            maxi = formula.var_app[(-1)*i];
            var = (-1)*i;
        }
    }
    return var;
}

int moms(Formula formula) {

    /// returns literal with most occurrences in clauses of minimum size
    // find minimum size of clauses
    int mini = std::numeric_limits<int>::max();
    for(auto& cls: formula.clauses)
        if(cls.nr_literals - cls.nr_literals_false < mini && cls.flag == 0)
            mini = cls.nr_literals - cls.nr_literals_false;

    map<int, int> occurrences;
    for(int i=1;i<=formula.variables;i++) {
        occurrences[i] = 0;
        occurrences[i*(-1)] = 0;
    }
    // count occurrences of variables in the clauses of minimum size
    for(auto& cls: formula.clauses)
        if(cls.flag == 0 && cls.nr_literals - cls.nr_literals_false == mini)
            for(auto& lit: cls.lst)
                if(lit.flag == 0)
                    occurrences[lit.name]++;
    int maxi = 0, var;
    for(int i=1;i<=formula.variables;i++) {
        if(occurrences[i] > maxi) {
                maxi = occurrences[i];
                var = i;
        }
        if(occurrences[i*(-1)] > maxi) {
                maxi = occurrences[i*(-1)];
                var = i*(-1);
        }
    }
    return var;
}

int positMOMS(Formula formula) {
    /// returns variable with most occurrences in clauses of minimum size
    return abs(moms(formula));
}

int ZMmoms(Formula formula) {
    /// returns variable with most negative occurrences in clauses of minimum size
    return moms(formula)*(-1);
}

int JeroslowWang(Formula formula) {
    map<int, float> J;
    float maxi = -1;
    int var;
    for(int i=1;i<=formula.variables;i++) {
        J[i]=0;
        J[i * (-1)] = 0;
    }
    for(auto& cls: formula.clauses)
        if(cls.flag == 0)
            for(auto& lit: cls.lst)
                if(lit.flag == 0) {
                    J[lit.name] += pow(2, (-1) * (cls.nr_literals - cls.nr_literals_false));
                    //gg<<lit.name<<' '<<pow(2, (-1) * (cls.nr_literals - cls.nr_literals_false))<<'\n';
                }

    for(int i=1;i<=formula.variables;i++) {
        //gg<<i<<' '<<J[i]<<'\n';
        //gg<<(-1)*i<<' '<<J[(-1)*i]<<'\n';
        if(J[i] > maxi) {
            maxi = J[i];
            var = i;
        }
        if(J[(-1) * i] > maxi) {
            maxi = J[(-1) * i];
            var = (-1) * i;
        }
    }
    return var;
}

int JeroslowWangAbs(Formula formula) {
    return abs(JeroslowWang(formula));
}

int JeroslowWangNeg(Formula formula) {
    return (-1) * JeroslowWang(formula);
}

int VSIDS(Formula formula, int conflicts) {
    if(conflicts % 100 == 0) {
        for(int i=1; i<=formula.variables; i++) {
            if(formula.var_app[i] > 0)
                formula.var_app[i] = formula.var_app[i]/2;
            if(formula.var_app[i*(-1)] > 0)
                formula.var_app[i * (-1)] = formula.var_app[i * (-1)]/2;
        }
    }
    return pickMostPopular(formula);
}

int pickBranchingVariable(Formula formula, int nr_conflicts, int heuristic) {

    switch(heuristic){
        case 1:
            return pickFirstFound(formula);
            break;
        case 2:
            return pickMostPopular(formula);
            break;
        case 3:
            return moms(formula);
            break;
        case 4:
            return positMOMS(formula);
            break;
        case 5:
            return ZMmoms(formula);
            break;
        case 6:
            return JeroslowWang(formula);
            break;
        case 7:
            return VSIDS(formula, nr_conflicts);
            break;

    }
}

