/********************************************************************************
  Copyright 2017 Frank Imeson and Stephen L. Smith

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*********************************************************************************/


#include "formula.hpp"
using namespace formula;

#define DEBUG

/********************************************************************************
 * 
 * Functions
 * 
 ********************************************************************************/         


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_implies (const Lit &a, const Lit &b) {
  Formula* formula = new Formula(F_OR);
  formula->add(~a);
  formula->add(b);
  return formula;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_implies (const Lit &a, Formula* b) {
  Formula* formula = new Formula(F_OR);
  formula->add(~a);
  formula->add(b);
  return formula;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_implies (Formula *a, const Lit &b) {
  Formula* formula = new Formula(F_OR);
  a->negate();
  formula->add(a);
  formula->add(b);
  return formula;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_implies (Formula *a, Formula* b) {
  Formula* formula = new Formula(F_OR);
  formula->add(a->negate());
  formula->add(b);
  return formula;
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int formula::get_solution (const Solver &solver, vec<Lit> &lits, Var max_var) {
    if (max_var < 0)
        max_var = solver.model.size()-1;
    for (unsigned int i=0; i<=max_var; i++)
        lits.push( mkLit(i, solver.model[i] == l_False) );
    return 0;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_iff (const Lit &a, const Lit &b) {
  Formula* formula = new Formula(F_OR);
  Formula* f0 = new Formula(F_AND);
  Formula* f1 = new Formula(F_AND);
  f0->add(a);
  f0->add(b);
  f1->add(~a);
  f1->add(~b);
  formula->add(f0);
  formula->add(f1);
  return formula;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_at_least_one_in_a_set (const vector<int> &var_list) {
    Formula* formula = new Formula(F_OR);
    for (int i=0; i<var_list.size(); i++)
        formula->add(mkLit(var_list[i], false));
    return formula;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_one_in_a_set (const vector<int> &var_list) {
  Formula* formula = new Formula(F_OR);
  for (int i=0; i<var_list.size(); i++) {
    Formula* f0 = new Formula(F_AND);
    for (int j=0; j<var_list.size(); j++) {
      if (i == j)
        f0->add(mkLit(var_list[j], false));
      else
        f0->add(mkLit(var_list[j], true));
    }
    formula->add(f0);
  }
  return formula;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula* formula::constraint_at_most_one_in_a_set (const vector<int> &var_list) {
  Formula* formula = new Formula(F_OR);

  for (int i=0; i<var_list.size(); i++) {
    Formula* f0 = new Formula(F_AND);
    for (int j=0; j<var_list.size(); j++) {
      if (i == j)
        f0->add(mkLit(var_list[j], false));
      else
        f0->add(mkLit(var_list[j], true));
    }
    formula->add(f0);
  }

  Formula* f0 = new Formula(F_AND);
  for (int i=0; i<var_list.size(); i++) {
    f0->add(mkLit(var_list[i], true));
  }
  formula->add(f0);

  return formula;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int formula::negate_solution (const vec<Lit> &lits, Solver &solver) {
    vec<Lit> nlits;
    for (unsigned int i=0; i<lits.size(); i++)
        nlits.push( ~lits[i] );
    solver.addClause(nlits);
    return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Lit formula::translate (const int &lit) {
    return mkLit(abs(lit)-1, (lit<0));
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int formula::translate (const Lit &lit) {
    return sign(lit) ? -var(lit)-1:var(lit)+1;
}



/************************************************************//**
 * @brief	
 * @return            string representation of connective	
 * @version						v0.01b
 ****************************************************************/
string formula::str (const Con &connective) {
    switch (connective) {
        case F_AND:
            return "^";
        case F_OR:
            return "v";
        default:
            return " ";
    }
}



/************************************************************//**
 * @brief	
 * @return            string representation of connective	
 * @version						v0.01b
 ****************************************************************/
string formula::str (const Lit &lit) {
    stringstream out;
    if (sign(lit))
        out << "-";
    else
        out << " ";
    out << (var(lit)+1);
    return out.str();
}




/************************************************************//**
 * @brief	
 * @return            string representation of connective	
 * @version						v0.01b
 ****************************************************************/
string formula::str (const vec<Lit> &lits) {
    string result;
    for (unsigned int i=0; i<lits.size(); i++)
      result += str(lits[i]) + " ";
    return result;
}



/************************************************************//**
 * @brief	
 * @return            negated connective, -1 on error
 * @version						v0.01b
 ****************************************************************/
Con formula::negate (const Con &connective) {
    switch (connective) {
        case F_AND:
            return F_OR;
        case F_OR:
            return F_AND;
        default:
            return -1;
    }
}



/************************************************************//**
 * @brief
 * @return            negated litteral	
 * @version						v0.01b
 ****************************************************************/
Lit formula::negate (const Lit &lit) {
    return ~lit;
}



/********************************************************************************
 * 
 * Formula Class
 *
 ********************************************************************************/         




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula::~Formula () {
    clear();
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula::Formula ()
    : connective(F_AND)
    , max_var(0)
{}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Formula::Formula (Con _connective)
    : connective(_connective)
    , max_var(0)
{}



/************************************************************//**
 * @brief	            Clear all contents of Formula recursively
 * @version						v0.01b
 ****************************************************************/
int Formula::clear () {
    try {
        for (unsigned int i=0; i<formuli.size(); i++) {
            if (formuli[i] != NULL) {
                delete formuli[i];
                formuli[i] = NULL;
            }
        }
        formuli.clear();
    } catch(...) {
        perror("error Formula::clear(): ");
        exit(1);
    }
	return 0;
}



/************************************************************//**
 * @brief	            Negate entire formula
 * @return            0 on succession, < 0 on error
 * @version						v0.01b
 ****************************************************************/
int Formula::negate () {

    connective = ::negate(connective);
    if (connective < 0)
        return connective;
    
    for (unsigned int i=0; i<lits.size(); i++)
        lits[i] = ::negate(lits[i]);
        
    for (unsigned int i=0; i<formuli.size(); i++) {
        int rtn = formuli[i]->negate();
        if (rtn < 0)
            return rtn;
    }
    return 0;
}






/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Formula::track_max (const Var &var) {
    if (var > max_var)
        max_var = var;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int Formula::add (const Lit &lit) {
    track_max(var(lit));
    lits.push(lit);
    return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int Formula::add (const int &lit) {
    return add(translate(lit));
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int Formula::add (const vec<Lit> &lits) {
    for (unsigned int i=0; i<lits.size(); i++)
        add(lits[i]);
    return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int Formula::add (Formula *formula) {
    track_max(formula->maxVar());
    if (formula != NULL)
        formuli.push(formula);
    return 0;
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Var Formula::newVar () {
    max_var++;
    return max_var;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Var Formula::maxVar () {
    return max_var;
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int Formula::export_cnf (Lit &out, Formula *formula, Solver *solver) {

    if ( (formula == NULL && solver == NULL) || (formula != NULL && solver != NULL) )
        return -1;

    // setup vars in solver
    // translation requires a new var to replace nested phrases
    if (solver != NULL) {
        while (solver->nVars() < max_var+1)
            solver->newVar();
        out = mkLit( solver->newVar(), false );
    } else {
        while (formula->maxVar() < max_var)
            formula->newVar();
        out = mkLit( formula->newVar(), false );
    }

    vec<Lit> big_clause;
    switch (connective) {
        case F_OR:

            // Variables
            for (unsigned int i=0; i<lits.size(); i++) {
                big_clause.push(lits[i]);
                if (solver != NULL) {
                    solver->addClause(~lits[i], out);
                } else {
                    Formula *phrase = new Formula(F_OR);
                    phrase->add(~lits[i]);
                    phrase->add(out);
                    formula->add(phrase);
                }
            }

            // Nested formuli
            for (unsigned int i=0; i<formuli.size(); i++) {
                Lit cnf_out;
                if (int err = formuli[i]->export_cnf(cnf_out, formula, solver) < 0) 
                    return err;
                big_clause.push(cnf_out);
                if (solver != NULL) {
                    solver->addClause(~cnf_out, out);
                } else {
                    Formula *phrase = new Formula(F_OR);
                    phrase->add(~cnf_out);
                    phrase->add(out);
                    formula->add(phrase);
                }
            }

            big_clause.push(~out);
            break;
    
        case F_AND:

            // Variables
            for (unsigned int i=0; i<lits.size(); i++) {
                big_clause.push(~lits[i]);
                if (solver != NULL) {
                    solver->addClause(lits[i], ~out);
                } else {
                    Formula *phrase = new Formula(F_OR);
                    phrase->add(lits[i]);
                    phrase->add(~out);
                    formula->add(phrase);
                }
            }

            // Nested formuli
            for (unsigned int i=0; i<formuli.size(); i++) {
                Lit cnf_out;
                if (int err = formuli[i]->export_cnf(cnf_out, formula, solver) < 0) 
                    return err;
                big_clause.push(~cnf_out);
                if (solver != NULL) {
                    solver->addClause(cnf_out, ~out);
                } else {
                    Formula *phrase = new Formula(F_OR);
                    phrase->add(cnf_out);
                    phrase->add(~out);
                    formula->add(phrase);                    
                }

            }            

            big_clause.push(out);
            break;
    
        default:
            break;
    }

    if (solver != NULL) {
        solver->addClause(big_clause);
    } else {
        Formula *phrase = new Formula(F_OR);
        phrase->add(big_clause);
        formula->add(phrase);         
    }

    return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string Formula::str () {
    return str(string(""));
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string Formula::str (string prefix) {
    string result = prefix;

    if (formuli.size() == 0) {
//        result += "( ";
        for (unsigned int i=0; i<lits.size(); i++)
            result += ::str(lits[i]) + " " + ::str(connective) + " ";
        result.resize(result.size()-2);
//        result += ")\n";
        result += "\n";
    } else {
        result += prefix + ::str(connective) + '\n';

        for (unsigned int i=0; i<lits.size(); i++)
            result += prefix + " " + ::str(lits[i]) + '\n';        

        for (unsigned int i=0; i<formuli.size(); i++)
            result += formuli[i]->str(prefix+" ");
    }

    return result;
}






