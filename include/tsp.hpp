
/****************************************************************************//**
 ********************************************************************************
 * @file        tsp.hpp
 * @brief       
 * @author      Frank Imeson
 * @date        
 *
 * todo:
  * implement MST
  * implement upper bound for LKH (???)

 ********************************************************************************
 ********************************************************************************/

#ifndef TSP_H		// guard
#define TSP_H

/********************************************************************************
 * INCLUDE
 ********************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <boost/regex.hpp>

#include "formula.hpp"
#include "minisat/core/Solver.h"
#include "minisat/core/Dimacs.h"

using namespace std;
using namespace Minisat;

/********************************************************************************
 * Defs
 ********************************************************************************/
#define INF 999999


/********************************************************************************
 * Prototypes
 ********************************************************************************/

class TSP;
class MST;

int parse_input(string filename, TSP &graph);
//void tsp2sat(graph);



class TSP {
  private:
    int edge_var_offset;
    int _metric, _symmetric, _tsp_monotonic;
    string lkh_parameters;
  public:
    TSP () : _metric(-1), _symmetric(-1), _tsp_monotonic(-1), tsp_cost_budget(INF), subgraph_cost_budget(INF) {
      lkh_parameters  = "PRECISION         = 10\n";
      lkh_parameters += "MOVE_TYPE         = 5\n";
      lkh_parameters += "PATCHING_C        = 3\n";
      lkh_parameters += "PATCHING_A        = 2\n";
      lkh_parameters += "RUNS              = 1\n";
      lkh_parameters += "TRACE_LEVEL       = 0\n";
    };
    string name, type;
    int tsp_cost_budget, subgraph_cost_budget;
    vector< vector<bool> >  adjacency;
    vector< vector<int> > edge_weight;
    vector< vector<int> > vid_incoming_vars;
    vector< vector<int> > vid_outgoing_vars;
    vector<vector<int>> gtsp_sets, subgraphs;
    vector<int> level00_vars;
    vector<int> theory_vars;

    int size () {return edge_weight.size();};
    int tsp2cnf (Solver* solver);
    int eid2var (const int eid);
    int vid2var (const int vid);
    int var2eid (const int var);
    int var2vid (const int var);
    void eid2edge (const int eid, int &from, int &to);
    int edge2eid (const int from, const int to);
    int eid_cost (int eid);
    bool operator() (int eid00, int eid01) {return eid_cost(eid00) > eid_cost(eid01);};
    int  lit_cost (const Lit &lit);
    bool LKH(const vector<int> &vids, const int &max_cost, vector<int> &soln_tour, int &soln_cost);
    bool solve(const vector<int> &vids, const int tsp_cost_budget, const int subgraph_cost_budget, vector<int> &soln_tour, int &soln_cost);
    bool metric();
    bool symmetric();
    bool tsp_monotonic();
    void assume_metric() {_metric = 1;};
    void assume_non_metric() {_metric = 0;};
    void assume_symmetric() {_symmetric = 1;};
    void assume_non_symmetric() {_symmetric = 0;};
    void assume_tsp_monotonic() {_tsp_monotonic = 1;};
    void assume_non_tsp_monotonic() {_tsp_monotonic = 0;};
    void set_lkh_parameters(string input_file);
    string get_lkh_parameters();
    bool feasible(const vector<int> &soln_tour, const int tsp_cost_budget, const int subgraph_cost_budget);
    void split_vids(const vector<int> &vids, vector<vector<int>> &subgraph_vids);
    bool get_tour_cost(const vector<int> &tour, int &tsp_cost, int &max_subgraph_cost);
};


class MST {
  private:
    TSP*                    graph;
    int                     _size;
    int                     _cost;
    vector<bool>            subset;
    vector< vector<bool> >  adjacency;

    int                     z_vid;                // new vertex to insert
    vector<bool>            new_subset;
    vector< vector<bool> >  new_adjacency;

    void                    _insert(int r_vid, int &t_eid);
  public:
    MST (TSP* graph);
    MST (TSP* graph, const vector<int> subset);
    int insert(int vid);
    int adjacent (int vid, vector<int> &list);
    int size () {return _size;};
    int cost() {return _cost;};


    string str() {
      stringstream output;
      if (subset.size() > 0) {
        output << "[";
        for (int i=0; i<subset.size(); i++)
          if (subset[i])
            output << i << ", ";
        output << "]";
      } else {
        output << "[]";
      }
      return output.str();
    }
};



#endif
