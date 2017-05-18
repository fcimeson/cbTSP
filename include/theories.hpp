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

 ********************************************************************************/

// guard
#ifndef SMT_H
#define SMT_H

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
#include "formula.hpp"
#include "minisat/core/Solver.h"

using namespace std;
using namespace Minisat;


/********************************************************************************
 * Defs
 ********************************************************************************/



/********************************************************************************
 * Prototypes
 ********************************************************************************/


/************************************************************//**
 * @brief	                    Overide class
 * @version						v0.01b
 ****************************************************************/
class Theory {
//    friend class TSP_Theory;
//    friend class Edge_TSP_Theory;
//    friend class Metric_TSP_Theory;
//    friend class MST_Theory;
//    friend class Cardinality_EQ_Theory;
//    private:
//        bool conflict, conflict_checked;

    public:
        virtual void reset () {};
        virtual void print () = 0;
        virtual void minisat_trail_push_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit>& trail, 
            vec<Lit>& propagate_list)
        {};
        virtual void minisat_trail_shrink_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            int amount)
        {};
        virtual bool minisat_check_conflict_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            vec<Lit> &conflict_list)
        {};
};


/************************************************************//**
 * @brief	                    Overide class
 * @version						v0.01b
 ****************************************************************/
class TSP_Theory : public Theory {
    public:
        int cb_interval;
        int tsp_cost_budget, subgraph_cost_budget;
        double theory_time;
        vector<int> soln_tour;
        virtual void print () {printf("TSP_Theory\n");};
};


/************************************************************//**
 * @brief
 * @version						v0.01b
 ****************************************************************/
class Edge_TSP_Theory : public TSP_Theory {
    private:
        TSP *graph;
        int edge_costs;
        vector<int> soln_eids;
        bool conflict, conflict_checked;

    public:
        Edge_TSP_Theory (TSP *graph, int verbose_level=-1);
        virtual void print () {printf("Edge_TSP_Theory\n");};
        virtual void reset ();

        virtual void minisat_trail_push_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit>& trail, 
            vec<Lit>& propagate_list);

        virtual void minisat_trail_shrink_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            int amount);

        virtual bool minisat_check_conflict_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            vec<Lit> &conflict_list);
};


/************************************************************//**
 * @brief
 * @version						v0.01b
 ****************************************************************/
class Metric_TSP_Theory : public TSP_Theory {
    private:
        TSP *graph;
        vector<int> soln_vids;
        bool conflict, conflict_checked;

    public:
        Metric_TSP_Theory (TSP *graph, int verbose_level=-1);
        virtual void print () {printf("Metric_TSP_Theory\n");};
        virtual void reset ();

        virtual void minisat_trail_push_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit>& trail, 
            vec<Lit>& propagate_list);

        virtual void minisat_trail_shrink_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            int amount);

        virtual bool minisat_check_conflict_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            vec<Lit> &conflict_list);
};


/************************************************************//**
 * @brief
 * @version						v0.01b
 ****************************************************************/
class MST_Theory : public TSP_Theory {
    private:
        TSP *graph;
        MST *mst;
        vector<int> soln_vids;
        bool mst_valid;
        bool conflict, conflict_checked;

    public:
        int tsp_cost_budget, subgraph_cost_budget;
        MST_Theory (TSP *graph, int verbose_level=-1);
        virtual void print () {printf("MST_Theory\n");};

        virtual void reset ();

        virtual void minisat_trail_push_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit>& trail, 
            vec<Lit>& propagate_list);

        virtual void minisat_trail_shrink_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            int amount);

        virtual bool minisat_check_conflict_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            vec<Lit> &conflict_list);
};


/************************************************************//**
 * @brief
 * @version						v0.01b
 ****************************************************************/
class Cardinality_EQ_Theory : public Theory {
    private:
        vector<Lit> lit_set;
        vector<bool> var_in_set, var_lit_sign;
        int cardinality;
        bool conflict, conflict_checked;

    public:
        Cardinality_EQ_Theory (vector<Lit> lit_set, int cardinality, int verbose_level=-1);
        virtual void print () {printf("Cardinality_EQ_Theory\n");};

        virtual void minisat_trail_push_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit>& trail, 
            vec<Lit>& propagate_list);

        virtual void minisat_trail_shrink_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            int amount);

        virtual bool minisat_check_conflict_cb (
            const VMap<lbool> &assigns, 
            const vec<Lit> &trail, 
            vec<Lit> &conflict_list);
};


#endif




