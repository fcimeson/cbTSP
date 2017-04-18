
/****************************************************************************//**
 ********************************************************************************
 * @file        main.cpp
 * @brief       
 * @author      Frank Imeson
 * @date        
 * @TODO
 *              TSP parameters
 *              Callback paramter
 ********************************************************************************
 ********************************************************************************/

#include "main.hpp"

//#define DEBUG



/*****************************************************************************
 *****************************************************************************
 * 
 * Functions
 *         
 *****************************************************************************
 *****************************************************************************/




/*****************************************************************************
 * Main
 ****************************************************************************/
int main(int argc, char **argv) {


    /******************************
     * Setup
     ******************************/
    srand ( time(NULL) );
    bool      brute(false), print_lkh_params(false), tsp_monotonic(false), non_tsp_monotonic(false);
    int       max_time(-1), max_usat_time(-1), verbose_level(-1), bdiv_parameter(10), cb_interval(1);
    int       max_tsp_cost, max_subgraph_cost;
    string    tsp_filename, sat_filename, output_filename, lkh_parameter_filename, search_method;
    int64_t   conflict_budget, propagation_budget;

    /************************************************************
     * Command Line Parser
     ************************************************************/
    po::options_description optional_args("Usage input_files [options]");
    optional_args.add_options()
    ( "input",
      po::value< vector<string> >(),
      "input instance file(s)"
    )
    ( "output,o",
      po::value<string>(&output_filename)->default_value(""),
      "output filename"
    )
    ( "brute", 
      po::value(&brute)->zero_tokens(),
     "enable brute mode (cb_interval > |V|)"
    )
    ( "max_time",
      po::value<int>(&max_time),      
      "max trial time (seconds)"
    )
    ( "max_usat_time",
      po::value<int>(&max_usat_time),
      "max trial for each sat instance (seconds)"
    )
    ( "max_tsp_cost",
      po::value<int>(&max_tsp_cost)->default_value(-1),
      "max tsp cost"
    )
    ( "max_subgraph_cost",
      po::value<int>(&max_subgraph_cost)->default_value(-1),
      "max subgraph cost"
    )
    ( "search_method",
      po::value<string>(&search_method)->default_value("binary"),
      "choose between: linear, binary, adaptive"
    )
    ( "bdiv",
      po::value<int>(&bdiv_parameter)->default_value(10),
      "set binary search divider"
    )
    ( "cb_interval",
      po::value<int>(&cb_interval)->default_value(1),
      "tsp callback interval"
    )
    ( "conflicts",
      po::value<int64_t>(&conflict_budget)->default_value(-1),
      "(-1 for unlimited)"
    )
    ( "propagations",
      po::value<int64_t>(&propagation_budget)->default_value(-1),
      "(-1 for unlimited)"
    )
    ( "tsp_monotonic",
      po::value(&tsp_monotonic)->zero_tokens(),
      "assume graph is tsp monotonic"
    )
    ( "non_tspMonotonic",
      po::value(&non_tsp_monotonic)->zero_tokens(),
      "assume graph is not tsp monotonic (brute mode)"
    )
    ( "input_lkh_params",
      po::value<string>(&lkh_parameter_filename),
      "input LKH parameter file"
    )
    ( "print_lkh_params",
      po::value(&print_lkh_params)->zero_tokens(),
      "print default lkh parameters"
    )
    ( "verbose,v",
      po::value<int>(&verbose_level)->default_value(1),
      "verbose level"
    )
    ( "help,h",
      "produce help message"
    )
    ;
    po::positional_options_description positional_args;
    positional_args.add("input", -1);
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(optional_args).positional(positional_args).run(), vm);
        po::notify(vm);
    } catch(exception& e) {
        cerr << "error: " << e.what() << '\n';
        return 1;
    } catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    /******************************
     * Parse input
     ******************************/
    if (vm.count("help") or !vm.count("input")) {
        cout << optional_args << '\n';
        return 0;
    }
    
    if (vm["input"].as< vector<string> >().size() == 1) {
        sat_filename = vm["input"].as< vector<string> >()[0] + ".cnf";
        tsp_filename = vm["input"].as< vector<string> >()[0] + ".tsp";
    } else if (vm["input"].as< vector<string> >().size() == 2) {
        sat_filename = vm["input"].as< vector<string> >()[0];
        tsp_filename = vm["input"].as< vector<string> >()[1];
    } else {
        cout << optional_args << '\n';
        return 0;
    }

    if (verbose_level >= 1) {
        printf("input cnf: %s\n", sat_filename.c_str());
        printf("input tsp: %s\n", tsp_filename.c_str());
    }

    // setup problem instance
    SATTSP problem(sat_filename, tsp_filename, verbose_level);
    if (verbose_level >= 1) {
        printf("setup...\n");
    }
    problem.set_search_method(search_method);
    problem.set_bdiv_parameter(bdiv_parameter);
    if (brute or non_tsp_monotonic) {
        problem.set_search_method("linear");
        problem.set_bdiv_parameter(999999);
    }
    if (tsp_monotonic)
        problem.assume_tsp_monotonic();
    if (!problem.enable_lkh_theory()) {
        printf("Error: could not enable LKH theory. Is the instance tsp_monotonic?\n");
        return 0;
    }
    problem.setConfBudget(conflict_budget);
    problem.setPropBudget(propagation_budget);
    problem.set_cb_interval(cb_interval);
    if (max_usat_time > 0)
        problem.setUsatTimeBudget(max_usat_time);
    if (max_time > 0)
        problem.setTimeBudget(max_time);
    if (vm.count("input_lkh_params")) {
        problem.set_lkh_parameters(lkh_parameter_filename);
    }
    if (print_lkh_params) {
        cout << problem.get_lkh_parameters() << '\n';
    }

    // solve
    if (verbose_level >= 1) {
        printf("solving...\n");
    }
    bool result(false);
    if (max_tsp_cost >= 0) {
        result = problem.solve(max_tsp_cost, max_subgraph_cost);
        if (result) {
            problem.get_solution();
            assert (problem.verify_soln());
        }
    } else {
        result = problem.solve_optimal(verbose_level);
    }

    ofstream  out_file;
    streambuf *backup = cout.rdbuf();
    if (output_filename.length() > 0) {
        out_file.open(output_filename.c_str());
        cout.rdbuf(out_file.rdbuf());
    }
    cout << problem.output_conf();
    cout << problem.output_stats();
    cout << problem.output_solution();
    cout.rdbuf(backup);
    if (output_filename.length() > 0)
        out_file.close();
    return 0;
}
