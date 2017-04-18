
/****************************************************************************//**
 ********************************************************************************
 * @file        subisosat.hpp
 * @brief       
 * @author      Frank Imeson
 * @date        
 * @todo
 *   lkh after every solution
 *   edge+vertex theory (vertex priority)
 *   no solution
 ********************************************************************************
 ********************************************************************************/

#ifndef SAT_H		// guard
#define SAT_H

/********************************************************************************
 * INCLUDE
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "tsp.hpp"
#include "theories.hpp"
#include "formula.hpp"
#include "minisat/core/Solver.h"


using namespace std;
//using namespace boost::timer;
using namespace Minisat;

/********************************************************************************
 * Defs
 ********************************************************************************/
#ifdef VER100
    #define LIB_VER   1.00
#else
    #define LIB_VER   1.10
#endif
enum SEARCH_METHOD {LINEAR, BINARY, ADAPTIVE_BINARY};


/********************************************************************************
 * Prototypes
 ********************************************************************************/

class SATTSP {
  private:
    Solver              *original_formula;
    Solver              *formula;
    TSP                 *graph;
    MST                 *mst;
    string              sat_filename, tsp_filename;
    int                 verbose_level;
    bool                edge_theory, vertex_theory, lkh_theory, mst_theory, solving;
    double              reduction_time, solver_time, theory_time;
    int                 minisat_nDecisions, minisat_nLearnts;    
    int                 cb_minisat_error;
    int                 new_clauses;
    timeval             max_wall_time;
    int                 search_method, bdiv_parameter;

    vector< vector<Theory*> >   var_theories;
    vector<Theory*>             theories;
    TSP_Theory                  *tsp_theory;

    int                 soln_cost, tsp_cost_budget, subgraph_cost_budget;
    vector<int>         soln_tour;
    vector<bool>        soln_assigns;

    bool                clause_checked, vid_conflict, eid_conflict, mst_valid;
    vector<int>         soln_vids, soln_eids;
    int                 solver_time_budget, usat_time_budget;
    int64_t             conflict_budget, propagation_budget;

    static void minisat_trail_push_cb_wrapper (
        void* object_pointer, 
        const VMap<lbool> &assigns, 
        const vec<Lit>& trail, 
        vec<Lit>& propagate_list);

    static void minisat_trail_shrink_cb_wrapper (
        void* object_pointer, 
        const VMap<lbool> &assigns, 
        const vec<Lit> &trail, 
        int amount);

    static bool minisat_check_conflict_cb_wrapper (
        void* object_pointer, 
        const VMap<lbool> &assigns, 
        const vec<Lit> &trail, 
        vec<Lit> &conflict_list);

    static bool minisat_within_budget_cb_wrapper (
        void* object_pointer);

    void reset_formula();
    string output_solution (const vector<int> &tour, const vector<bool> &assigns);

  public:
    SATTSP (string sat_filename, string tsp_filename, int verbose_level=-1);

    bool solve (int tsp_cost_budget, int subgraph_cost_budget);
    bool solve_optimal (int verbose_level=2);

    string output_conf();
    string output_stats();
    string output_solution();
    void print_smt2(int tsp_cost_budget);
    void print_cnf_smt2();

    void get_solution ();
    bool verify_soln (const int &cost, const vector<int> &tour, const vector<bool> &assigns, int verbose_level=-1);
    bool verify_soln (int verbose_level=-1);

    bool enable_linear_search();
    bool enable_adaptive_search();
    bool enable_edge_theory();
    bool enable_lkh_theory();
    bool enable_mst_theory();

    bool set_bdiv_parameter(int x) {bdiv_parameter = x;}

    bool setConfBudget(int64_t x)         {conflict_budget    = x;}
    bool setPropBudget(int64_t x)         {propagation_budget = x;}
    void setTimeBudget(int x)             {solver_time_budget = x;}
    void setUsatTimeBudget(int x)         {usat_time_budget   = x;}
    void assume_metric() {graph->assume_metric();};
    void assume_non_metric() {graph->assume_non_metric();};
    void assume_symmetric() {graph->assume_symmetric();};
    void assume_non_symmetric() {graph->assume_non_symmetric();};
    void assume_tsp_monotonic() {graph->assume_tsp_monotonic();};
    void assume_non_tsp_monotonic() {graph->assume_non_tsp_monotonic();};
    void set_lkh_parameters(string input_file) {graph->set_lkh_parameters(input_file);};
    void set_cb_interval(int x) {tsp_theory->cb_interval = x;};
    string get_lkh_parameters() {return graph->get_lkh_parameters();};
    bool set_search_method(string method);
};

#endif
