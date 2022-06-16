#include <fstream>
#include <list>
#include "Clause.h"
#include <map>
#include <cstdlib>
#include "Formula.h"
#include "Variable.h"
#include "heuristics.h"
#include <string>
#include <chrono>
using namespace std::chrono;
using namespace std;

ofstream g ("output.out");

Formula readFormula(int variables, int clauses, map<int, Variable> &assignment, string file_name) {

    ifstream f (file_name);
    char c;
    string s;
    while(true) {
        f>>c;
        // if comment
        if(c == 'c')
            getline(f, s); //ignore
        else { //is 'p'
            f>>s; // 'cnf'
            break;
        }
    }
    f>>variables>>clauses;
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
    f.close();
    return formula;
}

void print(Formula formula) {

    for(Clause clause: formula.clauses) {
        //if(clause.flag == 0) //{
            g<<"clause "<<clause.index<<": ";
            for(literal lit: clause.lst) {
                //if(lit.flag == 0)
                    g<<lit.name<<' '<<lit.flag<<' ';
            }
            g<<'\n';
        }
    //}
}

void printAssignment(map<int, Variable> assignment, Formula formula) {

    for(int i=1; i<=formula.variables; i++) {
        if(assignment[i].value == 0)
            g<<i<<' ';
        else
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
                    assignment[abs(lit.name)].value = (lit.name > 0 ? 1 : 0);
                    if(clause.nr_literals > 1)
                        assignment[abs(lit.name)].antecedent = clause.index;
                    return lit.name;
                }
            }
    return 0;
}

int unit_propagation(Formula &formula, map<int, Variable> &assignment, int level, int unitLiteral) {

    /// if no literal given, find unit clause
    /// antecedent of decision variable is NULL (0)
    if(unitLiteral == 0)
        unitLiteral = find_unit_clause(formula, assignment);

    assignment[abs(unitLiteral)].value = (unitLiteral > 0)? 1 : -1;
    assignment[abs(unitLiteral)].level = level;
    //assignment[abs(unitLiteral)].antecedent = 0;

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
                return clause.index;
            }
        }

        unitLiteral = find_unit_clause(formula, assignment);
        if(unitLiteral) {
            assignment[abs(unitLiteral)].value = (unitLiteral > 0)? 1 : -1;
            assignment[abs(unitLiteral)].level = level;
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

void pureLiteral(Formula &formula, map<int,Variable> &assignment, int level) {

    int pureLit = 0;
    /// find pure literal
    for(int i=1;i<=formula.variables;i++) {
        if(formula.var_app[i] == 0 && formula.var_app[(-1)*i] > 0) {
            pureLit = (-1)*i;
            break;
        }
        if(formula.var_app[(-1)*i] == 0 && formula.var_app[i] > 0) {
            pureLit = i;
            break;
        }
    }
    /*
    for(auto& cls: formula.clauses)
        if(cls.flag == 0)
            for(auto& lit: cls.lst)
                if(formula.var_app[lit.name * (-1)] == 0) {
                    pureLit = lit.name;

                }
    */
    /// add to assignment
    //g<<"pure literal "<<pureLit<<'\n';
    if(pureLit) {
        Variable var;
        var.value = (pureLit < 0) ? -1 : 1;
        var.level = level;
        var.antecedent = 0;
        assignment[abs(pureLit)] = var;

        //g<<"pure literal "<<pureLit<<'\n';

        /// satisfy clauses that contain it
        for(auto& cls: formula.clauses) {
           if(cls.flag == 0)
                for(auto& lit: cls.lst)
                    if(lit.flag == 0 && lit.name == pureLit) {
                        cls.flag = level;
                        for(auto& lit2: cls.lst)
                            if(lit.flag == 0 && lit.name != pureLit)
                                formula.var_app[lit.name]--;
                        continue;
                    }
        }

        formula.var_app[pureLit] = 0;
    }
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

    //printAssignment(assignment, formula);
    //g<<"conlfict al level "<<conflictLevel<<'\n';
    //g<<"conlfict al clause "<<conflictIndex;
    Clause cls2, learntClause;
    int cnt = 0, lit, complementaryLit;
    learntClause = getClausefromIndex(conflictIndex, formula);
    /*
    g<<"Conflict clause; ";
    for(auto& lit: learntClause.lst)
        g<<lit.name<<' '<<lit.flag<<' '<<assignment[abs(lit.name)].antecedent<<' ';
    g<<'\n';*/
    do {
        cnt = 0;
        for(auto& l: learntClause.lst) {
            lit = abs(l.name);
            if(assignment[lit].level == conflictLevel)
                cnt++;
            if(assignment[lit].level == conflictLevel && assignment[lit].antecedent != 0) {
                cls2 = getClausefromIndex(assignment[lit].antecedent, formula);
                complementaryLit = lit;
                //g<<"complementray lit "<<complementaryLit<<'\n';
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
    g<<"c\n";
    g<<"Backtrack level: "<<backtrackLevel<<'\n';*/
    learntClause.index = ++formula.nrclauses;
    learntClause.nr_literals = learntClause.lst.size();
    for(auto& lit: learntClause.lst) {
        if(assignment[abs(lit.name)].value != 0)
            lit.flag = (lit.name * assignment[abs(lit.name)].value > 0)?
                            assignment[abs(lit.name)].level :
                            (-1)*assignment[abs(lit.name)].level;
            if(lit.flag < 0)
                learntClause.nr_literals_false++;
    }
    if(learntClause.nr_literals - learntClause.nr_literals_false == 1)
        for(auto lit: learntClause.lst)
            if(lit.flag == 0)
                assignment[abs(lit.name)].antecedent = learntClause.index;

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

string cdcl(Formula formula, map<int, Variable> &assignment, int heuristic) {
    int level = 1, conflict = 0, backtrackLevel, lit = 0, nr_conflicts = 0, restartInterval = 100;
    pureLiteral(formula, assignment, level);
    while(true) {
        //print(formula);
        //for(int i=1;i<=formula.variables;i++)
        //    g<<i<<' '<<formula.var_app[i]<<' '<<formula.var_app[(-1)*i]<<'\n';
        conflict = unit_propagation(formula, assignment, level, lit);
        if(conflict) {
            if(level == 1) return "UNSAT\n";
            backtrackLevel = conflictAnalysis(formula, assignment, conflict, level);
            if(nr_conflicts++ == restartInterval) {
                nr_conflicts = 0;
                restartInterval *= 1.5;
                backtrack(formula, assignment, 2);
                level = 2;
                lit = 0;
            }
            else {
                backtrack(formula, assignment, backtrackLevel);
                lit = 0;
                level = backtrackLevel;
            }
        }
        else {
            if(empty_formula(formula)) break;
            level++;
            lit = pickBranchingVariable(formula, nr_conflicts, heuristic);//, conflict);
            //g<<"decided to branch on "<<lit<<'\n';
        }
    }
    printAssignment(assignment, formula);
    return "SAT\n";
}

void tester() {

    list<string> files;
    files.push_back("input.in");
    //files.push_back("test2.cnf");

    for(string file: files) {

        g<<file<<'\n';
        for(int i=1;i<=7;i++) {
            Formula formula;
            int variables, clauses;
            map<int, Variable> assignment;
            formula = readFormula(variables, clauses, assignment, file);
            auto start = high_resolution_clock::now();
            cdcl(formula, assignment, i);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            g << duration.count()/1000.0<<'\n';
        }
    }
}

int main() {

    Formula formula;
    int variables, clauses;
    map<int, Variable> assignment;
    formula = readFormula(variables, clauses, assignment, "input.in");
    auto start = high_resolution_clock::now();
    g<<cdcl(formula, assignment, 3);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    g << duration.count()/1000.0<<'\n';
    //tester();
    return 0;
}
