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


#include "theories.hpp"

//#define DEBUG
//#define DEBUG00
//#define MINISAT_VERBOSE


/****************************************************************
 * Edge TSP Theory
 *
 *
 *
 ****************************************************************/

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Edge_TSP_Theory::Edge_TSP_Theory (TSP *graph, int verbose_level)
    : graph(graph)
{
    soln_eids.clear();
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Edge_TSP_Theory::reset() {
    edge_costs = 0;
    soln_eids.clear();
    conflict = false;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Edge_TSP_Theory::minisat_trail_push_cb (const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& propagate_list) {
    Lit push_lit = trail.last();
    int push_var = var(push_lit);
    int vid00, vid01, eid = graph->var2eid(push_var);
    graph->eid2edge(eid, vid00, vid01);

    // edge theory (cost conflicts)
    if (eid >= 0 && !sign(push_lit)) {
        #ifdef MINISAT_VERBOSE
            printf("Minisat::trail.push_eid(%d)\n", eid);
        #endif
        
        // add up edge cost
        soln_eids.push_back(eid);
        edge_costs += graph->lit_cost(push_lit);
        if (edge_costs > tsp_cost_budget) {
            conflict = true;
            conflict_checked = false;
            #ifdef MINISAT_VERBOSE
                printf("Minisat::trail.push_conflict(%d)\n", int(tsp_cost_budget));
            #endif
        } else {
            conflict = false;
            conflict_checked = true;
        }
    }

    #ifndef VER100
        // edge theory (propigations)
        if (!conflict && eid >= 0 && !sign(push_lit)) {
            // propigate outgoing edge theory (one_in_a_set)
            for (int i=0; i<graph->vid_outgoing_vars[vid00].size(); i++) {
                int edge_var = graph->vid_outgoing_vars[vid00][i];
                if (edge_var != push_var) {
                    Lit edge_lit = mkLit(edge_var, true);
                    propagate_list.push(edge_lit);
                    #ifdef MINISAT_VERBOSE
                        printf("Minisat::trail.push_propigate(%d)\n", edge_var);
                    #endif
                }
            }
            // propigate incoming edge theory (one_in_a_set)
            for (int i=0; i<graph->vid_incoming_vars[vid01].size(); i++) {
                int edge_var = graph->vid_incoming_vars[vid01][i];
                if (edge_var != push_var) {
                    Lit edge_lit = mkLit(edge_var, true);
                    propagate_list.push(edge_lit);
                    #ifdef MINISAT_VERBOSE
                        printf("Minisat::trail.push_propigate(%d)\n", edge_var);
                    #endif
                }
            }
        }
        
        // level0 vertex theory (propigations)
        if (!conflict && !sign(push_lit) && push_var >= graph->level00_vars.front() && push_var <= graph->level00_vars.back()) {
            for (int i=0; i<graph->level00_vars.size(); i++) {
                int vid_var = graph->level00_vars[i];
                Lit vid_lit = mkLit(vid_var, true);
                propagate_list.push(vid_lit);
                #ifdef MINISAT_VERBOSE
                    printf("Minisat::trail.push_propigate(%d)\n", vid_var);
                #endif
            }
        }
    #endif
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Edge_TSP_Theory::minisat_trail_shrink_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, int amount) {
    for (int i=0; i<amount; i++) {
        Lit lit  = trail[trail.size()-i-1];
        int eid = graph->var2eid(var(lit));
        if (!sign(lit) && eid >= 0) {
            edge_costs -= graph->lit_cost(lit);
            assert (soln_eids.size() > 0);
            soln_eids.pop_back();
            conflict = false;
        }
    }

    #ifdef MINISAT_VERBOSE
        printf("Minisat::trail.shrink(%d)\n", amount);
        if (soln_eids.size() > 0) {
            printf("  |- eid_trail.size() = %d\n", (int)soln_eids.size());
            printf("  |- eid_trail.last(%d)\n", soln_eids.back());
        } else {
            printf("  |- eid_trail.size() = 0\n");
        }
    #endif
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool Edge_TSP_Theory::minisat_check_conflict_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, vec<Lit> &conflict_list) {
    if (conflict_checked)
        return false;
    conflict_checked = true;
    conflict_list.clear();

    if (conflict) {
        for (int i=0; i < soln_eids.size(); i++) {
            Lit lit = mkLit(graph->eid2var(soln_eids[i]),true);
            conflict_list.push(lit);
        }

        #ifdef MINISAT_VERBOSE
            printf("Minisat::cost_conflict()\n");
            printf("  |- edge_costs = %d \n", int(edge_costs));
            printf("  |- tsp_cost_budget   = %d \n", int(tsp_cost_budget));
            printf("  |- negate(eids): ");
            for (int i=0; i < conflict_list.size(); i++) {
                int eid = graph->var2eid(var(conflict_list[i]));
                printf("%d ", eid);
            }
            printf("\n");
        #endif 
        return true;
    }
    return false;
}



/****************************************************************
 * Metric TSP Theory
 *
 *
 *
 ****************************************************************/

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Metric_TSP_Theory::Metric_TSP_Theory (TSP *graph, int verbose_level)
    : graph(graph)
{}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Metric_TSP_Theory::reset() {
    soln_vids.clear();
    conflict = false;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
 void Metric_TSP_Theory::minisat_trail_push_cb (const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& propagate_list) {
    Lit push_lit = trail.last();
    int push_vid = graph->var2vid(var(push_lit));;

    // if lit is positive and in set then propagate
    if (!sign(push_lit) && push_vid >= 0) {
        soln_vids.push_back(push_vid);
        #ifdef MINISAT_VERBOSE
            printf("Minisat::trail.push_vid(%d)\n", push_vid);
        #endif

        // solve LKH
        if (soln_vids.size() % cb_interval == 0) {
            double not_lkh_time;
            clock_t tic00 = clock();

            timeval tic,toc;
            gettimeofday (&tic, NULL);
            int soln_cost;
            soln_tour.clear();
            conflict = !graph->solve(soln_vids, tsp_cost_budget, subgraph_cost_budget, soln_tour, soln_cost);
            conflict_checked = false;
            gettimeofday (&toc, NULL);
            theory_time += toc.tv_sec - tic.tv_sec;
            theory_time += (double) (toc.tv_usec - tic.tv_usec) / 1000000;

            clock_t toc00 = clock();
            not_lkh_time += (double) (toc00-tic00) / CLOCKS_PER_SEC;
            theory_time -= not_lkh_time;
        }
    }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Metric_TSP_Theory::minisat_trail_shrink_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, int amount) {
    for (int i=0; i<amount; i++) {
        Lit lit  = trail[trail.size()-i-1];
        int vid = graph->var2vid(var(lit));
        if (!sign(lit) && vid >= 0) {
            soln_vids.pop_back();
            conflict = false;
        }
    }
    #ifdef MINISAT_VERBOSE
        printf("Minisat::trail.shrink(%d)\n", amount);
        if (soln_vids.size() > 0)
            printf("  |- vid_trail.last(%d)\n", soln_vids.back());
        else
            printf("  |- vid_trail.size() = 0\n");
    #endif
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool Metric_TSP_Theory::minisat_check_conflict_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, vec<Lit> &conflict_list) {
    if (conflict_checked)
    return false;
    conflict_checked = true;
    conflict_list.clear();

    if (conflict) {
        vector<int> negate_vids;
        vector<vector<int>> sub_tours(graph->subgraphs.size());
        graph->split_vids(soln_tour, sub_tours);
        for (int i=0; i<graph->subgraphs.size(); i++){
            if (!graph->feasible(sub_tours[i], tsp_cost_budget, subgraph_cost_budget)) {
                negate_vids = sub_tours[i];
            }
        }
        if (negate_vids.size() == 0) {
            negate_vids = soln_vids;
        }
        assert (negate_vids.size() > 0);
        for (int i=0; i < negate_vids.size(); i++) {
            Lit lit = mkLit(graph->vid2var(negate_vids[i]),true);
            conflict_list.push(lit);
        }
//        #define MINISAT_VERBOSE00
        #ifdef MINISAT_VERBOSE00
            bool print(true);
            vector<int> test_vector = {0,2,6,15,17,19,29,31,38};
            for (int i=0; i < negate_vids.size(); i++) {
                int vid = negate_vids[i] % 40;
                if (!std::binary_search(test_vector.begin(), test_vector.end(), vid)) {
                    print = false;
                    break;
                }
            }            
            if (print) {            
                printf("Minisat::cost_conflict()\n");
                printf("  |- negate(vids): ");
                for (int i=0; i < negate_vids.size(); i++) {
                    printf("%d ", negate_vids[i]);
                }
                printf("\n");
                int cost;
                vector<int> soln;
                conflict = !graph->solve(soln_vids, tsp_cost_budget, subgraph_cost_budget, soln, cost);
                printf("  |- Max TSP cost: %d\n", tsp_cost_budget);
                printf("  |- Max Subgraph cost: %d\n", subgraph_cost_budget);
                printf("  |- Soln cost: %d\n", cost);
            }
        #endif
//        #define MINISAT_VERBOSE
        #ifdef MINISAT_VERBOSE
            printf("Minisat::cost_conflict()\n");
            printf("  |- negate(vids): ");
            for (int i=0; i < negate_vids.size(); i++) {
                printf("%d ", negate_vids[i]);
            }
            printf("\n");
            int cost;
            vector<int> soln;
            conflict = !graph->solve(soln_vids, tsp_cost_budget, subgraph_cost_budget, soln, cost);
            printf("  |- Max TSP cost: %d\n", tsp_cost_budget);
            printf("  |- Max Subgraph cost: %d\n", subgraph_cost_budget);
            printf("  |- Soln cost: %d\n", cost);
        #endif 
        return true;
    }
    return false;
}


/****************************************************************
 * MST Theory
 *
 *
 *
 ****************************************************************/

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
MST_Theory::MST_Theory (TSP *graph, int verbose_level)
    : graph(graph)
{}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void MST_Theory::reset() {
    delete mst;
    mst = new MST(graph);
    mst_valid = true;
    soln_vids.clear();
    conflict = false;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void MST_Theory::minisat_trail_push_cb (const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& propagate_list) {
    Lit push_lit = trail.last();
    int push_vid = graph->var2vid(var(push_lit));;

    // if lit is positive and in set then propagate
    if (!sign(push_lit) && push_vid >= 0) {
        soln_vids.push_back(push_vid);
        #ifdef MINISAT_VERBOSE
            printf("Minisat::trail.push_vid(%d)\n", push_vid);
        #endif

        // solve MST
        clock_t tic = clock();
        if (mst_valid) {
            mst->insert(push_vid);
            assert (mst->size() == soln_vids.size());
            #ifdef DEBUG
                MST test_mst(graph, soln_vids);
                printf("\n");
                printf("MST::insert(%d)\n", push_vid);
                printf("  actual            = %f\n", test_mst.cost());
                printf("  mst->cost()       = %f\n", mst->cost());
                printf("  mst->cost()*2     = %f\n", 2*mst->cost());
                printf("  subgraph_cost_budget = %f\n", subgraph_cost_budget);
                printf("  mst_vids          = %s\n", mst->str().c_str());
                printf("  actual_mst_vids   = %s\n", test_mst.str().c_str());
                assert (test_mst.cost() == mst->cost());
            #endif
        } else {
            delete mst;
            mst       = new MST(graph, soln_vids);
            mst_valid = true;
        }
        conflict = (2*mst->cost() > tsp_cost_budget);
        clock_t toc = clock();
        theory_time += (double)(toc-tic)/CLOCKS_PER_SEC;
    }
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void MST_Theory::minisat_trail_shrink_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, int amount) {
    for (int i=0; i<amount; i++) {
        Lit lit  = trail[trail.size()-i-1];
        int vid = graph->var2vid(var(lit));
        if (!sign(lit) && vid >= 0) {
            soln_vids.pop_back();
            conflict = false;
        }
    }
  
    #ifdef MINISAT_VERBOSE
        printf("Minisat::trail.shrink(%d)\n", amount);
        if (soln_vids.size() > 0)
            printf("  |- vid_trail.last(%d)\n", soln_vids.back());
        else
            printf("  |- vid_trail.size() = 0\n");
    #endif
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool MST_Theory::minisat_check_conflict_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, vec<Lit> &conflict_list) {
    if (conflict_checked)
    return false;
    conflict_checked = true;
    conflict_list.clear();

    if (conflict) {
        for (int i=0; i < soln_vids.size(); i++) {
            Lit lit = mkLit(graph->vid2var(soln_vids[i]),true);
            conflict_list.push(lit);
        }

        #ifdef MINISAT_VERBOSE
            printf("Minisat::cost_conflict()\n");
            printf("  |- negate(vids): ");
            for (int i=0; i < soln_vids.size(); i++) {
                int vid = graph->vid2var(soln_vids[i]);
                printf("%d ", vid);
            }
            printf("\n");
        #endif 
        return true;
    }
    return false;
}




/****************************************************************
 * Cardinality Equals Theory
 *
 *
 *
 ****************************************************************/

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
Cardinality_EQ_Theory::Cardinality_EQ_Theory (vector<Lit> lit_set, int cardinality, int verbose_level)
    : lit_set(lit_set)
    , cardinality(cardinality)
{
    assert (cardinality == 1);
    for (int i=0; i<lit_set.size(); i++) {
        int x = var(lit_set[i]);
        while (x >= var_in_set.size()) {
            var_in_set.push_back(false);
            var_lit_sign.push_back(false);
        }
        var_in_set[x] = true;
        var_lit_sign[x] = sign(lit_set[i]);
    }
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Cardinality_EQ_Theory::minisat_trail_push_cb (const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& propagate_list) {
    Lit push_lit = trail.last();
    int push_var = var(push_lit);

    // if lit satisfies set then propagate
    if (push_var < var_in_set.size() and var_in_set[push_var] and var_lit_sign[push_var] == sign(push_lit)) {        
        for (int i=0; i<lit_set.size(); i++) {
            Lit set_lit = lit_set[i];
            int set_var = var(set_lit);
            if (set_var != push_var) {
                propagate_list.push(~set_lit);
            }
        }
    }

    #if defined(DEBUG00) && defined(__GXX_EXPERIMENTAL_CXX0X__)
        if (push_var < var_in_set.size() and var_in_set[push_var] and var_lit_sign[push_var] == sign(push_lit)) {
            string s;
            if (!sign(push_lit))
                s += "  Card_EQ.push    x" + to_string(push_var);
            else
                s += "  Card_EQ.push   -x" + to_string(push_var);
            s += '\n';
            s += "  Card_EQ.set  = [";
            for (int i=0; i<lit_set.size(); i++) {
                Lit set_lit = lit_set[i];
                int set_var = var(set_lit);
                if (!sign(set_lit))
                    s += "x" + to_string(set_var) + ",";
                else
                    s += "-x" + to_string(set_var) + ",";
            }
            s = s.substr(0, s.size()-1);
            s += "] \n";
            s += "  Card_EQ.prop = [";
            for (int i=0; i<propagate_list.size(); i++) {
                Lit set_lit = propagate_list[i];
                int set_var = var(set_lit);
                if (!sign(set_lit))
                    s += "x" + to_string(set_var) + ",";
                else
                    s += "-x" + to_string(set_var) + ",";
            }
            s = s.substr(0, s.size()-1);
            s += "] \n";
            cout << s;
        }
    #endif
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void Cardinality_EQ_Theory::minisat_trail_shrink_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, int amount) {
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool Cardinality_EQ_Theory::minisat_check_conflict_cb (const VMap<lbool> &assigns, const vec<Lit> &trail, vec<Lit> &conflict_list) {
    return false;
}





