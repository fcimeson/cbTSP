
/****************************************************************************//**
 ********************************************************************************
 * @file        formula.hpp
 * @brief       
 * @author      Frank Imeson
 * @date        
 ********************************************************************************
 ********************************************************************************/

#ifndef FORMULA_H		// guard
#define FORMULA_H

/********************************************************************************
 * INCLUDE
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "minisat/core/Solver.h"

namespace formula {
using namespace Minisat;
using namespace std;

/********************************************************************************
 * Defs
 ********************************************************************************/

typedef int Con;
enum { F_AND, F_OR, F_XOR, F_NXOR };


/********************************************************************************
 * Prototypes
 ********************************************************************************/

class Formula;

Lit translate  (const int &lit);
int translate  (const Lit &lit);
string str (const Lit &literal);
string str (const Con &connective);
string str (const vec<Lit> &lits);
Lit negate (const Lit &literal);
Con negate (const Con &connective);
int get_solution    (const Solver &solver, vec<Lit> &lits, Var max_var = -1);
int negate_solution (const vec<Lit> &lits, Solver &solver);

Formula* constraint_iff(const Lit &a, const Lit &b);
Formula* constraint_implies(const Lit &a, const Lit &b);
Formula* constraint_implies(const Lit &a, Formula* b);
Formula* constraint_implies(Formula* a, const Lit &b);
Formula* constraint_implies(Formula* a, Formula* b);
Formula* constraint_at_least_one_in_a_set(const vector<int> &var_list);
Formula* constraint_one_in_a_set(const vector<int> &var_list);
Formula* constraint_at_most_one_in_a_set(const vector<int> &var_list);


/********************************************************************************
 * Classes
 ********************************************************************************/

class Formula {
    private:
        Var max_var;
        Con connective;
        vec<Formula*> formuli;
        vec<Lit> lits;

        void track_max( const Var &var);
    public:
        Formula  ();
        Formula  (Con relation);        
        ~Formula ();

        void set (Con _connective) {connective = _connective;}
        int clear ();    
        int negate ();
        Var newVar ();
        Var maxVar ();
        int add  (const int &lit);
        int add  (const Lit &lit);
        int add  (const vec<Lit> &lits);
        int add  (Formula* formula);
        int size () { return lits.size() + formuli.size(); };
        int export_cnf (Lit &out, Formula *formula = NULL, Solver *solver = NULL);
        string str ();
        string str (const string indent);
};



} // end namespace
#endif
