
/****************************************************************************//**
 ********************************************************************************
 * @file        
 * @brief       
 * @author      Frank Imeson
 * @date        
 ********************************************************************************
 ********************************************************************************/

#include "sattsp.hpp"

//#define DEBUG
//#define DEBUG00
//#define MINISAT_VERBOSE



/*****************************************************************************
 *****************************************************************************
 * 
 * Functions
 *         
 *****************************************************************************
 *****************************************************************************/




/************************************************************//**
 * @brief	
 * @version						v0.01b
 * TODO: make INF an input argument
 ****************************************************************/
SATTSP::SATTSP(string sat_filename, string tsp_filename, int verbose_level)
  : sat_filename(sat_filename)
  , tsp_filename(tsp_filename)
  , verbose_level(verbose_level)
  , reduction_time(0)
  , solver_time(0)
  , soln_cost(999999)
  , tsp_cost_budget(999999)
  , subgraph_cost_budget(999999)
  , conflict_budget(-1)
  , propagation_budget(-1)
  , clause_checked(false)
  , eid_conflict(false)
  , vid_conflict(false)
  , mst_valid(true)
  , edge_theory(false)
  , vertex_theory(false)
  , lkh_theory(false)
  , mst_theory(false)
  , tsp_theory(NULL)
  , cb_minisat_error(-1)
  , solver_time_budget(-1)
  , usat_time_budget(-1)
  , new_clauses(0)
  , bdiv_parameter(10)
  , minisat_nDecisions(0)
  , minisat_nLearnts(0)
  , search_method(BINARY)
{
  formula           = new Solver();
  original_formula  = new Solver();
  graph             = new TSP();

  time_t tic = time(0);
  try {
    gzFile cnf_file;
    cnf_file = gzopen(sat_filename.c_str(), "rb");
    parse_DIMACS(cnf_file, *original_formula);
    gzclose(cnf_file);
    cnf_file = gzopen(sat_filename.c_str(), "rb");
    parse_DIMACS(cnf_file, *formula);
    gzclose(cnf_file);
    parse_input(tsp_filename, *graph);
  } catch(exception& e) {
    cerr << "error: " << e.what() << '\n';
  }
  time_t toc = time(0);
  reduction_time += (double)(toc-tic); 

  formula->callback_obj_pt          = this;
  formula->trail_push_callback      = &SATTSP::minisat_trail_push_cb_wrapper;
  formula->trail_shrink_callback    = &SATTSP::minisat_trail_shrink_cb_wrapper;
  formula->check_conflict_callback  = &SATTSP::minisat_check_conflict_cb_wrapper;
  mst                               = new MST(graph);
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::set_search_method(string method) {
    if (method == "linear") {
        search_method = LINEAR;
    } else if (method == "binary") {
        search_method = BINARY;
    } else if (method == "adaptive") {
        search_method = ADAPTIVE_BINARY;
    } else {
        cerr << "Error: search method not supported\n";
        exit(0);
    }
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::enable_edge_theory()
{
    if (vertex_theory)   return false;
    time_t tic = time(0);
    graph->tsp2cnf(formula);
    time_t toc = time(0);
    reduction_time += (double)(toc-tic);
    tsp_theory = new Edge_TSP_Theory(graph);
    theories.push_back(tsp_theory);
    for (int i=0; i<graph->theory_vars.size(); i++) {
        int theory_var = graph->theory_vars[i];
        while (theory_var >= var_theories.size()) var_theories.push_back(vector<Theory*>());
        var_theories[theory_var].push_back(theories.back());
    }
    edge_theory = true;
    return true;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::enable_lkh_theory()
{
  if (edge_theory)              return false;
  if (!graph->tsp_monotonic())  return false;
  tsp_theory = new Metric_TSP_Theory(graph);
  theories.push_back(tsp_theory);
  // TODO: populate var_theories
  lkh_theory    = true;
  vertex_theory = true;
  return true;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::enable_mst_theory()
{
  if (edge_theory)          return false;
  if (!graph->metric())     return false;
  if (!graph->symmetric())  return false;
  tsp_theory = new MST_Theory(graph);
  theories.push_back(tsp_theory);
  // TODO: populate var_theories
  mst_theory    = true;
  vertex_theory = true;
  return true;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::reset_formula ()
{
    time_t tic = time(0);
    minisat_nDecisions += formula->decisions;
    minisat_nLearnts += formula->num_learnts;
    delete formula;
    formula = new Solver();
    gzFile cnf_file = gzopen(sat_filename.c_str(), "rb");
    parse_DIMACS(cnf_file, *formula);
    gzclose(cnf_file);

    formula->callback_obj_pt          = this;
    formula->trail_push_callback      = &SATTSP::minisat_trail_push_cb_wrapper;
    formula->trail_shrink_callback    = &SATTSP::minisat_trail_shrink_cb_wrapper;
    formula->check_conflict_callback  = &SATTSP::minisat_check_conflict_cb_wrapper;
    formula->within_budget_callback   = &SATTSP::minisat_within_budget_cb_wrapper;
    formula->random_seed              = rand();
    formula->rnd_init_act             = true;
    formula->random_var_freq          = 0.01; // in [0,1]

    if (edge_theory)
        graph->tsp2cnf(formula);

    for (int i=0; i<theories.size(); i++)
        theories[i]->reset();

    // catch variables that have already been assigned
    for (int var=0; var < original_formula->nVars(); var++) {
        if (formula->value(var) != l_Undef) {
            VMap<lbool> assigns;
            vec<Lit> trail;
            vec<Lit> propagate_list;
            if (formula->value(var) == l_True)
                trail.push(mkLit(var, false));
            else
                trail.push(mkLit(var, true));
            for (int i=0; i<theories.size(); i++)
                theories[i]->minisat_trail_push_cb(assigns, trail, propagate_list);
            assert (propagate_list.size() == 0);
        }
    }
  
  time_t toc = time(0);
  reduction_time += (double)(toc-tic);
  
  #ifdef DEBUG
    printf("\n\n\n  Max cost %d\n", tsp_cost_budget);
  #endif
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::minisat_trail_push_cb_wrapper (void* _sattsp_ptr, const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& propagate_list) {
    SATTSP* sattsp_ptr = (SATTSP*) _sattsp_ptr;
    Lit push_lit = trail.last();
    int push_var = var(push_lit);
    propagate_list.clear();
    if (sattsp_ptr->tsp_theory != NULL) {
        sattsp_ptr->tsp_theory->minisat_trail_push_cb(assigns, trail, propagate_list);
    }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::minisat_check_conflict_cb_wrapper (void* _sattsp_ptr, const VMap<lbool> &assigns, const vec<Lit>& trail, vec<Lit>& conflict_list) {
    SATTSP* sattsp_ptr = (SATTSP*) _sattsp_ptr;
    if (sattsp_ptr->tsp_theory != NULL) {
        bool conflict = sattsp_ptr->tsp_theory->minisat_check_conflict_cb(assigns, trail, conflict_list);
        #if defined(MINISAT_VERBOSE) && defined(__GXX_EXPERIMENTAL_CXX0X__)
            if (conflict_list.size()) {
                string s = "  conflict_list  = [";
                for (int i=0; i<conflict_list.size(); i++) {
                    Lit lit = conflict_list[i];
                    int x = var(lit);
                    if (!sign(lit))
                        s += "x" + to_string(x) + ",";
                    else
                        s += "-x" + to_string(x) + ",";
                }
                s = s.substr(0, s.size()-1);
                s += "] \n";
                cout << s;
            }
        #endif        
        if (conflict != CRef_Undef) sattsp_ptr->new_clauses++;
        return conflict;
    }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::minisat_trail_shrink_cb_wrapper (void* _sattsp_ptr, const VMap<lbool> &assigns, const vec<Lit>& trail, int amount) {
    SATTSP* sattsp_ptr = (SATTSP*) _sattsp_ptr;
    if (sattsp_ptr->tsp_theory != NULL) {
        sattsp_ptr->tsp_theory->minisat_trail_shrink_cb(assigns, trail, amount);
    }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::minisat_within_budget_cb_wrapper (void* _sattsp_ptr) {
    SATTSP* sattsp_ptr = (SATTSP*) _sattsp_ptr;

    // time budget
    timeval toc;
    gettimeofday (&toc, NULL);
    if (sattsp_ptr->max_wall_time.tv_sec >= 0 && toc.tv_sec > sattsp_ptr->max_wall_time.tv_sec) {
        return false;
    }

    return true;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 * TODO: confirm binary search logic
 ****************************************************************/
bool SATTSP::solve_optimal (int verbose_level)
{
    int           test_min(0), test_cost(0), test_max(soln_cost);
    int           bin_div(bdiv_parameter);    
    if (search_method == ADAPTIVE_BINARY) {
        bin_div = 2;
    }
    test_cost   = test_max - floor(test_max/bin_div);
    solving     = true;
    
    #ifdef DEBUG
        cout << "solve_optimal():\n";
        cout << "  tsp_cost_budget = " << tsp_cost_budget << '\n';
        cout << "  subgraph_cost_budget = " << tsp_cost_budget << '\n';
    #endif   

    // search
    reset_formula();    // hack!
    while (true) {
        if (verbose_level >= 100)
            printf("\ntest_min=%d, test_cost=%d, test_max=%d\n", test_min, test_cost, test_max);
        #ifdef DEBUG45
            cout << "SATTSP::solve_optimal():\n";
            cout << "  test_cost = " << test_cost << ", min = " << test_min << ", max = " << test_max << '\n';
        #endif
        bool solved(false);
        if (graph->type == "MIN_MAX_TSP") {
            solved = solve(tsp_cost_budget, test_cost);
        } else {
            solved = solve(test_cost, subgraph_cost_budget);
        }
        if (solved) {
            get_solution();
            // TODO: allow for solutions of size 3 and less
            assert (verify_soln());
            if (soln_tour.size() == 0)
                cerr << "Error: zero sized tour\n";
            test_max = soln_cost - 1;
            if (verbose_level == 1)
                cout << "+";
            if (verbose_level >= 2) {
                printf("\nFound New Soln:\n");
                printf("  soln.cost(): %d\n", soln_cost);
                printf("  soln.len():  %d\n", int(soln_tour.size()));
                printf("  soln.time(): %d\n", int(solver_time));
            }
            if (verbose_level == -3) {
                cout << output_conf();
                cout << output_stats();
                cout << output_solution();
            }
        } else {
            if (search_method == ADAPTIVE_BINARY) {
                bin_div = min(bin_div+1, bdiv_parameter);
            }
            test_min = test_cost + 1;
            if (verbose_level == 1)
                cout << "o";
            if (verbose_level >= 2) {
                cout << ".";
            }
            // reset formula
            reset_formula();
        }
        cout.flush();
        if (search_method == BINARY || search_method == ADAPTIVE_BINARY) {
            test_cost = test_max - floor((test_max-test_min)/bin_div);
        } else {
            assert (search_method == LINEAR);
            test_cost = test_max;
        }
        // this is wierd, but sometimes it will find a solution smaller than it previously couldn't
        if (test_min > test_max) {
            if (verbose_level >= 100)
                printf("\nDone: test_min=%d, test_cost=%d, test_max=%d\n", test_min, test_cost, test_max);
            break;
        }
    }
    printf("\n");

    solving = false;
    if (soln_tour.size() > 0)
        return true;
    else
        return false;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::solve (int _tsp_cost_budget, int _subgraph_cost_budget)
{
    // time budget: wall clock required since external calls are made to LKH
    timeval tic;
    gettimeofday (&tic, NULL);
    tsp_theory->theory_time = 0;
    if (usat_time_budget > 0) {
        max_wall_time.tv_sec = tic.tv_sec + usat_time_budget;
    } else {
        max_wall_time.tv_sec = -1;
    }
    if (solver_time_budget > 0 && (max_wall_time.tv_sec < 0 || max_wall_time.tv_sec > tic.tv_sec + solver_time_budget - solver_time)) {
        max_wall_time = tic;
        max_wall_time.tv_sec += (solver_time_budget - solver_time);
        if (tic.tv_sec > max_wall_time.tv_sec) {
            return false;
        }
    }
    
    // setup
    vec<Lit> assumptions;
    tsp_cost_budget = min(_tsp_cost_budget,graph->tsp_cost_budget);
    subgraph_cost_budget = min(_subgraph_cost_budget, graph->subgraph_cost_budget);
    tsp_theory->tsp_cost_budget = tsp_cost_budget;
    tsp_theory->subgraph_cost_budget = subgraph_cost_budget;
    bool result(false);
    reset_formula();    // hack!
    if (conflict_budget >=0)    formula->setConfBudget    (conflict_budget);
    if (propagation_budget >=0) formula->setPropBudget    (propagation_budget);

    #ifdef DEBUG
        cout << "solve():\n";
        cout << "  tsp_cost_budget = " << tsp_cost_budget << '\n';
        cout << "  subgraph_cost_budget = " << tsp_cost_budget << '\n';
    #endif   

    if (edge_theory) {
        result = (formula->solveLimited(assumptions) == l_True);
        result = (formula->solveLimited(assumptions) == l_True);
    }

    if (vertex_theory) {
        while (true) {
            result = (formula->solveLimited(assumptions) == l_True);
                if (result) {
                    int cost(0);
                    vector<int> solution, vids;
                    for (int vid=0; vid< graph->size(); vid++) {
                        if ( formula->model[graph->vid2var(vid)] == l_True) {
                            vids.push_back(vid);
                        }
                    }
                    if (mst_theory) {
                        if (2*mst->cost() <= tsp_cost_budget) {
                            #ifdef DEBUG
                            printf("\n");
                            printf("MST::solve\n");
                            printf("  mst->cost()   = %f\n", mst->cost());
                            printf("  mst->cost()*2 = %f\n", 2*mst->cost());
                            printf("  tsp_cost_budget  = %f\n", tsp_cost_budget);
                            printf("  mst_vids      = %s\n", mst->str().c_str());
                            printf("  vids.len      = %d\n", int(vids.size()));
                            #endif

                            break;
                        } else {
                            #ifdef DEBUG
                                printf("\n");
                                printf("MST::solve -- negate\n");
                            #endif
                            vec<Lit> negate;
                            for (int i=0; i<vids.size(); i++) {
                                negate.push(mkLit(graph->vid2var(vids[i]), true));
                            }
                            formula->addClause(negate);
                            result = false;

                            #ifdef MINISAT_VERBOSE
                                printf("Minisat::cost_conflict()\n");
                                printf("  |- negate(vids): ");
                                for (int i=0; i < vids.size(); i++) {
                                    printf("%d ", vids[i]);
                                }
                                printf("\n");
                            #endif 
                        }
                    }

                    if (lkh_theory) {
                        graph->solve(vids, tsp_cost_budget, subgraph_cost_budget, solution, cost);
                        if (cost <= tsp_cost_budget) {
                            break;
                        } else {
                            vec<Lit> negate;
                            for (int i=0; i<vids.size(); i++) {
                                negate.push(mkLit(graph->vid2var(vids[i]), true));
                            }
                            formula->addClause(negate);
                            result = false;

                            #ifdef MINISAT_VERBOSE
                                printf("Minisat::cost_conflict()\n");
                                printf("  |- negate(vids): ");
                                for (int i=0; i < vids.size(); i++) {
                                    printf("%d ", vids[i]);
                                }
                                printf("\n");
                            #endif 
                        }
                    }

            } else {
                break;    
            }
        }
    }

    timeval toc;
    gettimeofday (&toc, NULL);
    solver_time += toc.tv_sec - tic.tv_sec;
    solver_time += (double) (toc.tv_usec - tic.tv_usec) / 1000000;
    theory_time += tsp_theory->theory_time;
    if (formula->callback_error >= 0) {
        cb_minisat_error = formula->callback_error;
        formula->callback_error = -1;
    }
    return result;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::verify_soln (const int &cost, const vector<int> &tour, const vector<bool> &assigns, int verbose_level)
{
    int count(0);
    for (int var=0; var < original_formula->nVars(); var++) {
        if (formula->model[var] == l_True && graph->var2vid(var) >= 0) {
            count++;
        }
    }
    if (count == 0 ) {
        if (verbose_level > 0) {
            printf("SATTSP::verify_soln()\n");
            printf("  |- tour.size()   == 0\n");
        }
        return false;
    }
    if (count != tour.size()) {
        if (verbose_level > 0) {
            printf("SATTSP::verify_soln()\n");
            printf("  |- tour.size()   != Number of include vertices in F\n");
            printf("  |- tour.size()    = %d\n", int(tour.size()));
            printf("  |- |F.vids()==T|  = %d\n", count);
        }
        #ifdef __GXX_EXPERIMENTAL_CXX0X__
            if (verbose_level >= 10) {
                string s = "  |- tour = [" + to_string(tour[0]);
                for (int i=1; i<tour.size(); i++) {
                    s += "," + to_string(tour[i]);
                }
                s += "]\n";
                printf("%s", s.c_str());
                vector<int> vids;            
                for (int var=0; var < original_formula->nVars(); var++) {
                    int vid = graph->var2vid(var);
                    if (formula->model[var] == l_True && vid >= 0) {
                        vids.push_back(vid);
                    }
                }
                s = "  |- F.vids() = [" + to_string(vids[0]);
                for (int i=1; i<vids.size(); i++) {
                    s += "," + to_string(vids[i]);
                }
                s += "]\n";
                printf("%s", s.c_str());
                vector< pair<int,int> > edges;
                for (int var=0; var < formula->nVars(); var++) {
                    int eid = graph->var2eid(var);
                    int from, to;
                    graph->eid2edge(eid, from, to);
                    if (formula->model[var] == l_True && eid >= 0) {
                        edges.push_back(pair<int,int>(from,to));
                    }
                }
                if (edges.size() > 0) {
                    s = "  |- F.edges() = [<" + to_string(edges[0].first) + "," + to_string(edges[0].second) + ">";
                    for (int i=1; i<vids.size(); i++) {
                        s += ",<" + to_string(edges[i].first) + "," + to_string(edges[i].second) + ">";
                    }
                    s += "]\n";
                    printf("%s", s.c_str());
                } else {
                    s = "  |- F.edges() = [}\n";
                    printf("%s", s.c_str());
                }
            }
        #endif
        return false;
    }

    for (int i=0; i < tour.size(); i++) {
        int var = graph->vid2var(tour[i]);
        if (formula->model[var] == l_False) {
            if (verbose_level > 0) {
                printf("SATTSP::verify_soln()\n");
                printf("  |- vid(%d) is in tour but not sign(%d) = F\n", var, var);
            }
            return false;
        }
    }

    int cost_calc(0);
    for (int i=1; i < tour.size(); i++) {
        int vid00 = tour[i-1];
        int vid01 = tour[i];
        cost_calc += graph->edge_weight[vid00][vid01];
    }
    int vid00 = tour.back();
    int vid01 = tour[0];
    cost_calc += graph->edge_weight[vid00][vid01];
    if (cost_calc != cost) {
        if (verbose_level > 0) {
            printf("SATTSP::verify_soln()\n");
            printf("  |- soln cost != tour cost\n");
            printf("  |- soln cost  = %d\n", cost);
            printf("  |- tour cost  = %d\n", cost_calc);
        }
        return false;
    }

    vec<Lit> assumptions;
    for (int var=0; var<original_formula->nVars(); var++) {
        assumptions.push(mkLit(var, formula->model[var] == l_False));
    }
    if (original_formula->solveLimited(assumptions) == l_False) {
        if (verbose_level > 0) {
            printf("SATTSP::verify_soln()\n");
            printf("  |- sat assignment does not satisfiy orginal formula\n");
        }
        return false;
    }
    return true;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool SATTSP::verify_soln (int verbose_level)
{
  return verify_soln (soln_cost, soln_tour, soln_assigns, verbose_level);
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::get_solution ()
{

  soln_cost = 0;
  soln_tour.clear();
  soln_assigns.clear();

  if (edge_theory) {
    vector<int> edge_set;
    for (int i=0; i<graph->size(); i++)
      edge_set.push_back(-1);

    // populate cost, edge set and assigments
    for (int var=0; var < formula->nVars(); var++) {
      if (var < original_formula->nVars())
        soln_assigns.push_back(formula->model[var] == l_True);
      if (formula->model[var] == l_True && graph->var2eid(var) >= 0) {
        int from, to, eid = graph->var2eid(var);
        graph->eid2edge(eid, from, to);
        edge_set[from] = to;
        soln_cost += graph->lit_cost(mkLit(graph->eid2var(eid)));
      }
    }

    // populate tour
    bool first(true);
    for (int i=0; i < edge_set.size(); i++) {
      if (edge_set[i] >= 0) {
        soln_tour.push_back(i);
        first = false;
        break;
      }
    }
    assert (!first);
    int counter(0);
    while (true) {
      int vid = edge_set[soln_tour.back()];
      assert (vid >= 0);
      if (vid == soln_tour[0])
        break;
      else
        soln_tour.push_back(vid);
      assert (counter < edge_set.size());
      counter++;
    }
  }

  if (vertex_theory) {
    // populate cost, vid set and assigments
    vector<int> vid_set;
    for (int var=0; var < original_formula->nVars(); var++) {
      soln_assigns.push_back(formula->model[var] == l_True);
      if (formula->model[var] == l_True && graph->var2vid(var) >= 0) {
        vid_set.push_back(graph->var2vid(var));
      }
    }
    graph->solve(vid_set, tsp_cost_budget, subgraph_cost_budget, soln_tour, soln_cost);
    #ifdef DEBUG
        cout << "SATTSP::get_solution():\n";
        cout << "  cost = " << soln_cost << '\n';
        cout << "  tour = ";
        for (int i=0; i < soln_tour.size(); i++) {
            cout << " " << i;
        }
        cout << '\n';
    #endif
  }

}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string SATTSP::output_conf ()
{
    char buffer00[1024];
    gethostname(buffer00, 1024);
    string hostname(buffer00);

    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char buffer01[80];
    strftime(buffer01,80,"%d.%m.%Y",timeinfo);
    string date(buffer01);

    stringstream output;
    output << "\n";
    output << "Configuration:\n";
    output << boost::format("  Host:                %s\n")     %(hostname);
    output << boost::format("  Date:                %s\n")     %(date);
    if (edge_theory)
        output << boost::format("  Reduction:           HAM\n");
    else
        output << boost::format("  Reduction:           None\n");
    if (edge_theory)
        output << boost::format("  Name:                cb_edge\n");
    else if (lkh_theory)
        output << boost::format("  Name:                cb_lkh\n");
    output << boost::format("  Solver:              miniSMT\n");
    output << boost::format("  Edge Theory:         %s\n")     % (edge_theory?"Enabled":"Disabled");
    output << boost::format("  LKH Theory:          %s\n")     % (lkh_theory?"Enabled":"Disabled");
    output << boost::format("  MST Theory:          %s\n")     % (mst_theory?"Enabled":"Disabled");
    switch (search_method) {
        case LINEAR:
            output << boost::format("  Search:              Linear\n");
            break;
        case BINARY:
            output << boost::format("  Search:              Binary\n");
            break;
        case ADAPTIVE_BINARY:
            output << boost::format("  Search:              Adaptive_Binary\n");
            break;
    }
    if (search_method == BINARY || search_method == ADAPTIVE_BINARY)
        output << boost::format("  B-Search Divider:    %d\n")     % (bdiv_parameter);
    output << boost::format("  Library:             sattsp.cpp\n");
    output << boost::format("  Version:             v%0.2f\n") % (LIB_VER);
    output << boost::format("  Conflict Budget:     %ld\n")    % (conflict_budget);
    output << boost::format("  Propagation Budget:  %ld\n")    % (propagation_budget);
    if (solving)
        output << boost::format("  State:               Solving\n");
    output << boost::format("  Max USAT Time:       %ds\n")    % (usat_time_budget);
    output << boost::format("  Max Time:            %ds\n")    % (solver_time_budget);
    
    return output.str();
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string SATTSP::output_stats ()
{
    double minisat_time = solver_time - theory_time;
    minisat_nDecisions += formula->decisions;
    minisat_nLearnts += formula->num_learnts;
    vector<string> sat_tokens, tsp_tokens;
    boost::split(sat_tokens, sat_filename, boost::is_any_of("/"));
    boost::split(tsp_tokens, tsp_filename, boost::is_any_of("/"));
    stringstream output;
    output << "\n";
    output << "Statistics:\n";
    output << boost::format("  Formula:             %s\n") % sat_tokens.back();
    output << boost::format("  Graph:               %s\n") % tsp_tokens.back();
    output << boost::format("  Vars:                %f\n") % original_formula->nVars();
    output << boost::format("  Clauses:             %f\n") % original_formula->nClauses();
    output << boost::format("  newClauses:          %f\n") % new_clauses;
    output << boost::format("  Vertices:            %f\n") % graph->size();
    output << boost::format("  Reduction Time:      %f\n") % reduction_time;
    output << boost::format("  Minisat Time:        %f\n") % minisat_time;
    output << boost::format("    nDecisions:        %d\n") % minisat_nDecisions;
    output << boost::format("    nLearnts:          %d\n") % minisat_nLearnts;
    output << boost::format("  LKH Time:            %f\n") % theory_time;
    output << boost::format("  Solver Time:         %f\n") % solver_time;
    if (solver_time_budget > 0 && solver_time > solver_time_budget)
        output << boost::format("  Timed Out:           True\n");
    if (cb_minisat_error >= 0)
        output << boost::format("  cb_minisat:          Faulted\n");
    return output.str();
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string SATTSP::output_solution (const vector<int> &tour, const vector<bool> &assigns)
{
    stringstream output;
    output << "\n";
    output << "Solution:\n";
    if (tour.size() > 0) {
        // output costs
        int cost(0), tsp_cost(0), max_subgraph_cost(0);
        graph->get_tour_cost(tour, tsp_cost, max_subgraph_cost);
        if (graph->type == "MIN_MAX_TSP") {
            cost = max_subgraph_cost;
        } else {
            cost = tsp_cost;
        }
        output << boost::format("  Cost = %d\n") % cost;
        output << boost::format("  TSP Cost = %d\n") % tsp_cost;
        output << boost::format("  Max Subgraph Cost = %d\n") % max_subgraph_cost;

        // output tour(s)
        vector<vector<int>> sub_tour;
        graph->split_vids(tour, sub_tour);
        for (int i=0; i<sub_tour.size(); i++) {
            output << "  Tour = [";
            if (sub_tour[i].size() > 0) {
                output << boost::format("%d") % sub_tour[i][0];
            }
            for (int j=1; j<sub_tour[i].size(); j++) {
                output << boost::format(",%d") % sub_tour[i][j];
            }
            output << "]\n";
        }
        #ifdef OUTPUT_TOUR
            output << boost::format("  Tour = [%d") % tour[0];
            for (int j=1; j<tour.size(); j++) {
                output << boost::format(",%d") % tour[j];
            }
            output << "]\n";
        #endif

        output << boost::format("  Vars = [%s") % (assigns[0]?"T":"F");
        for (int i=1; i < original_formula->nVars(); i++) {
            output << boost::format(",%s") % (assigns[i]?"T":"F");
        }
        output << "]\n";
    } else {
        output << boost::format("  Cost = -1.00\n");
        output << boost::format("  Tour = []\n");
        output << boost::format("  Vars = []\n");
    }
    return output.str();
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string SATTSP::output_solution ()
{
  return output_solution(soln_tour, soln_assigns);
}






/*****************************************************************************
 *****************************************************************************
 * 
 * Debug/Testing
 *         
 *****************************************************************************
 *****************************************************************************/



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::print_smt2 (int tsp_cost_budget)
{
  assert (edge_theory && !vertex_theory);
  #define QF_LIA
  #ifdef QF_LIA
    cout << "(set-logic QF_LIA)\n";
  #else
    cout << "(set-logic QF_UF)\n";
  #endif
  cout << "(set-info :smt-lib-version 2.0)\n";
  cout << "(set-option :produce-models true)\n";
  cout << '\n';
  
  for (int var00=0; var00<formula->nVars(); var00++) {
    cout << "(declare-fun x" << var00 << " () Bool)\n";
  }
  cout << '\n';
  
  #ifdef QF_LIA
    for (int vid00=0; vid00<graph->size(); vid00++) {
      for (int vid01=0; vid01<graph->size(); vid01++) {
        int eid = graph->edge2eid(vid00, vid01);
        int     var = graph->eid2var(eid);
        int cost = graph->edge_weight[vid00][vid01];
        cout << "(define-fun c" << var << " () Int (ite x" << var << ' ' << cost << " 0))\n";
      }
    }
    cout << '\n';
  
    cout << "(define-fun cost () Int (+ ";
    for (int vid00=0; vid00<graph->size(); vid00++) {
      for (int vid01=0; vid01<graph->size(); vid01++) {
        int eid = graph->edge2eid(vid00, vid01);
        int     var = graph->eid2var(eid);
        cout << " c" << var;
      }
    }
    cout << "))\n";
    cout << '\n';
  #endif
  
  cout << "(assert (and\n";
  ClauseIterator ci = formula->clausesBegin();
  while (ci != formula->clausesEnd()) {
    cout << " (or";
    for (int i=0; i<ci.operator*().size(); i++) {
      if (sign(ci.operator*()[i]))
        cout << " (not x" << var(ci.operator*()[i]) << ')';
      else
        cout << " x" << var(ci.operator*()[i]);
    }
    cout << ")\n";
    ci.operator++();
  }
  cout << "))\n\n";

  #ifdef QF_LIA
    cout << "(assert (<= cost " << tsp_cost_budget << "))\n";
    cout << "(check-sat)\n";
    cout << "(get-value (cost";
  #else
    cout << "(check-sat)\n";
    cout << "(get-value (";
  #endif
  for (int var=0; var<formula->nVars(); var++) {
    cout << " x" << var;
  }
  cout << "))\n";
  cout << "(exit)\n";
  cout << '\n';
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void SATTSP::print_cnf_smt2 ()
{
  cout << "(set-logic QF_UF)\n";
  cout << "(set-info :smt-lib-version 2.0)\n";
  cout << "(set-option :produce-models true)\n";
  cout << '\n';
  
  for (int var00=0; var00<original_formula->nVars(); var00++) {
    cout << "(declare-fun x" << var00 << " () Bool)\n";
  }
  cout << '\n';
  
  ClauseIterator ci = original_formula->clausesBegin();
  while (ci != original_formula->clausesEnd()) {
    cout << "(assert (or";
    for (int i=0; i<ci.operator*().size(); i++) {
      if (sign(ci.operator*()[i]))
        cout << " (not x" << var(ci.operator*()[i]) << ')';
      else
        cout << " x" << var(ci.operator*()[i]);
    }
    cout << "))\n";
    ci.operator++();
  }

  cout << "(check-sat)\n";
  cout << "(get-value (";
  for (int var=0; var<original_formula->nVars(); var++) {
    cout << " x" << var;
  }
  cout << "))\n";
  cout << "(exit)\n";
  cout << '\n';
}

