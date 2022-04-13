#include <fstream>
#include <list>
#include "Clause.h"
#include <map>
#include <cstdlib>
#include "Formula.h"
#include "Variable.h"
using namespace std;

ifstream f ("input.in");
ofstream g ("output.out");

Formula readFormula(int variables, int clauses, map<int, Variable> &assignment) {

    Formula formula(variables, clauses);
    int x;
    for(int i=0;i<clauses;i++) {
        Clause cls;
        cls.index = i+1;
        int nr = 0;
        do {
            f>>x;
            literal lit;
            lit.name = x;
            if(x!=0) {
                cls.lst.push_back(lit);
                formula.var_app[x]++;
                nr++;
            }
        }while(x!=0);
        cls.nr_literals = nr;
        formula.clauses.push_back(cls);
    }
    for(int i=0;i<=variables;i++)
        assignment[i].level = 0;

    return formula;
}

void print(Formula formula) {

    for(Clause clause: formula.clauses) {
        if(clause.flag == 0) {
            g<<"clause "<<clause.index<<": ";
            for(literal lit: clause.lst) {
                if(lit.flag == 0)
                    g<<lit.name<<' '<<lit.flag<<' ';
            }
            g<<'\n';
        }
    }
}

void printAssignment(map<int, Variable> assignment, Formula formula) {

    for(int i=1; i<=formula.variables; i++) {
        g<<assignment[i].value * i<<' ';
        if(i % 10 == 0)
            g<<'\n';
    }
    g<<'\n';
}

int find_unit_clause(Formula &formula, map<int, Variable> &assignment) {
    /// search for a clause that only has one literal

    for(Clause clause: formula.clauses)
        if(clause.flag == 0 && clause.nr_literals - clause.nr_literals_false == 1)
            for(auto& lit: clause.lst) {
                if(lit.flag == 0) {
                    if(lit.name > 0) {
                        assignment[lit.name].value = 1;
                        assignment[lit.name].antecedent = clause.index;
                    }
                    else {
                        assignment[(-1)*lit.name].value = -1;
                        assignment[(-1)*lit.name].antecedent = clause.index;
                    }
                    return lit.name;
                }
            }
    return 0;
}

int unit_propagation(Formula &formula, map<int, Variable> &assignment, int level, int unitLiteral) {

    /// if no literal given, find unit clause
    /// antecedent of decision variable is NULL (0)
    if(!unitLiteral) {
        unitLiteral = find_unit_clause(formula, assignment);
        if(unitLiteral > 0) {
            assignment[unitLiteral].level = level;
        }
        else {
            assignment[(-1)*unitLiteral].level = level;
        }
    }
    else {
        if(unitLiteral > 0) {
            assignment[unitLiteral].value = 1;
            assignment[unitLiteral].level = level;
            assignment[unitLiteral].antecedent = 0;
        }
        else {
            assignment[(-1)*unitLiteral].value = -1;
            assignment[(-1)*unitLiteral].level = level;
            assignment[(-1)*unitLiteral].antecedent = 0;
        }
    }

    /// find implied literals
    /// clauses that contain it are satisfied
    /// negation gets deleted
    while(unitLiteral) {
        //print(formula);
        formula.var_app[unitLiteral] = 0;
        formula.var_app[(-1)*unitLiteral] = 0;

        for(auto& clause: formula.clauses) {

            if(clause.flag == 0)
                for(auto& lit: clause.lst) {
                    if(lit.flag == 0 && lit.name == (-1)*unitLiteral) {
                        lit.flag = (-1)*level;
                        clause.nr_literals_false++;
                    }
                    else if(lit.flag == 0 && lit.name == unitLiteral){
                        lit.flag = level;
                        clause.flag = level;
                        for(auto& lit2: clause.lst)
                            if(lit2.name != unitLiteral && lit2.flag == 0){
                                formula.var_app[lit2.name]--;
                                lit2.flag = level;
                            }
                        }
                    }
            /// check if there is a conflict and return index of that clause
            if(clause.nr_literals_false == clause.nr_literals) {
                //g<<"conflict at clause "<<clause.index<<'\n';
                return clause.index;
            }
        }
        unitLiteral = find_unit_clause(formula, assignment);
        if(unitLiteral) {
            if(unitLiteral > 0) {
                assignment[unitLiteral].level = level;
            }
            else {
                assignment[(-1)*unitLiteral].level = level;
            }
        }
    }
    /// if no conflict return 0
    return 0;

}

bool AllVariablesAssigned(Formula formula, map<int, Variable> assignment) {

    for(int i=1; i<=formula.variables; i++)
        if(assignment[i].value == 0)
            return 0;
    return 1;

}

int pickBranchingVariable(Formula formula, map<int, Variable> assignment) {

    /// for now, branching is done on the first literal found
    for(auto& cls: formula.clauses)
        if(cls.flag == 0)
            for(auto& lit: cls.lst)
                if(lit.flag == 0)
                    return lit.name;
}

bool empty_clause (Formula formula) {

    for(auto& clause: formula.clauses) {
        if(clause.flag == 0 && clause.nr_literals == clause.nr_literals_false)
            return true;
    }
    return false;
}

bool empty_formula(Formula formula) {

    for(auto& clause: formula.clauses)
        if(clause.flag == 0)
            return false;
    return true;
}

Clause resolution(Clause cls1, Clause cls2, Formula formula, int complementaryLit) {
    Clause resolvent;
    for(auto& lit: cls1.lst)
        if(abs(lit.name) !=  abs(complementaryLit))
            resolvent.lst.push_back(lit);
    for(auto& lit: cls2.lst)
        if(abs(lit.name) !=  abs(complementaryLit))
            resolvent.lst.push_back(lit);
    resolvent.lst.sort();
    resolvent.lst.unique();
    return resolvent;

}

Clause getClausefromIndex(int index, Formula formula) {
    for(auto& cls: formula.clauses)
        if(cls.index == index)
            return cls;
}

int conflictAnalysis(Formula &formula, map<int, Variable> assignment, int conflictIndex, int conflictLevel) {
    /// analyse conflict
    /// learn clause to avoid it happening again
    /// add it to the formula
    /// return the level to backtrack to

    Clause cls2, learntClause;
    int cnt = 0, lit, complementaryLit;
    learntClause = getClausefromIndex(conflictIndex, formula);

    do {
        cnt = 0;
        for(auto& l: learntClause.lst) {
            lit = abs(l.name);
            if(assignment[lit].level == conflictLevel)
                cnt++;
            if(assignment[lit].level == conflictLevel && assignment[lit].antecedent != 0) {
                cls2 = getClausefromIndex(assignment[lit].antecedent, formula);
                complementaryLit = lit;
            }
        }
        if(cnt == 1)
            break;
        learntClause = resolution(learntClause, cls2, formula, complementaryLit);
    }while(true);

    int backtrackLevel = 0;
    if(learntClause.lst.size() == 1)
        backtrackLevel = 1;
    else
    for(auto& lit: learntClause.lst) {
        if(assignment[abs(lit.name)].level > backtrackLevel && assignment[abs(lit.name)].level != conflictLevel)
            backtrackLevel = assignment[abs(lit.name)].level;
    }
    /*
    g<<"Learned clause:\n ";
    for(auto& lit:learntClause.lst)
        g<<lit.name<<' ';
    g<<'\n';*/
    //g<<"Backtrack level: "<<backtrackLevel<<'\n';
    learntClause.index = ++formula.nrclauses;
    for(auto& cls: formula.clauses)
        if(cls.index == formula.nrclauses)
            for(auto& lit: cls.lst) {
                lit.flag = assignment[abs(lit.name)].value * assignment[abs(lit.name)].level;
                if(assignment[abs(lit.name)].value < 0)
                    cls.nr_literals_false--;
            }

    formula.clauses.push_back(learntClause);

    return backtrackLevel;

}

void backtrack(Formula &formula, map<int, Variable> &assignment, int backtrackLevel) {
    /// return to 0 all flags that have the value of the level
    /// fix var_app
    /// delete variables in assignment chosen at a higher level
    for(auto& clause: formula.clauses) {
        for(auto& l: clause.lst) {
            if(l.flag > backtrackLevel ||
               (l.flag < 0 && l.flag < (-1)*backtrackLevel))
            {
                if(l.flag < 0)
                    clause.nr_literals_false--;
                l.flag = 0;
                formula.var_app[l.name]++;
            }
        }
        if(clause.flag > backtrackLevel)
            clause.flag = 0;
    }

    for(int i=1; i<=formula.variables; i++) {
        if(assignment[i].level > backtrackLevel) {
            assignment[i].level = 0;
            assignment[i].value = 0;
            assignment[i].antecedent = 0;
        }
    }
}

string cdcl(Formula formula, map<int, Variable> &assignment) {
    int level = 1, conflict = 0, backtrackLevel, lit = 0;

    while(true) {
        conflict = unit_propagation(formula, assignment, level, lit);
        if(conflict) {
            if(level == 1) return "UNSAT";
            backtrackLevel = conflictAnalysis(formula, assignment, conflict, level);
            backtrack(formula, assignment, backtrackLevel);
            lit = 0;
            level = backtrackLevel;
        }
        else {
            if(AllVariablesAssigned(formula, assignment)) break;
            level++;
            lit = pickBranchingVariable(formula, assignment);
            //g<<"decided to branch on "<<lit<<'\n';
        }
    }
    printAssignment(assignment, formula);
    return "SAT";
}

int main() {

    Formula formula;
    int variables, clauses;
    map<int, Variable> assignment;
    f>>variables>>clauses;
    formula = readFormula(variables, clauses, assignment);
    g<<cdcl(formula, assignment);

    return 0;
}
