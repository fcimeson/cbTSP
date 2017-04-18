
/****************************************************************************//**
 ********************************************************************************
 * @file        main.cpp
 * @brief       
 * @author      Frank Imeson
 * @date        
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
    Solver    cnf_solver;
    TSP       graph;
    bool      print_smt(false), cnf2smt(false), metric(true), non_metric(false), symmetric(false), non_symmetric(false);
    bool      edge_theory(false), lkh_theory(true), mst_theory(false), linear_search(false);
    int       num_trials, max_time(-1), max_usat_time(-1), verbose_level(-1), bin_search_divider(10);
    double    max_cost;
    string    tsp_filename, sat_filename, output_filename;
    int64_t   conflict_budget, propagation_budget;

    /************************************************************
     * Command Line Parser
     ************************************************************/
    po::options_description optional_args("Usage input_files [options]");
    optional_args.add_options()
    ("input",       po::value< vector<string> >(),  "input instance file(s)")
    ("trials",      po::value<int>(&num_trials),    "number of trials to conduct")
    ("max_time",    po::value<int>(&max_time),      "max trial time (seconds)")
    ("max_usat_time",
                    po::value<int>(&max_usat_time),      "max trial for each sat instance (seconds)")
    ("max_cost",    po::value<double>(&max_cost)->default_value(-1),
                                                    "max cost")
    ("bdiv",        po::value<int>(&bin_search_divider)->default_value(10),
                                                    "set binary search divider")
    ("conflicts",   po::value<int64_t>(&conflict_budget)->default_value(-1),
                                                    "(-1 for unlimited)")
    ("propagations",po::value<int64_t>(&propagation_budget)->default_value(-1),
                                                    "(-1 for unlimited)")
    ("edge",        po::value(&edge_theory)->zero_tokens(),
                                                    "enable EDGE theory")
    ("lkh",         po::value(&lkh_theory)->zero_tokens(),
                                                    "enable LKH theory")
    ("mst",         po::value(&mst_theory)->zero_tokens(),
                                                    "enable MST theory")
    ("linear_search",
                    po::value(&linear_search)->zero_tokens(),
                                                    "enable linear search")
    ("metric",      po::value(&metric)->zero_tokens(),
                                                    "assume graph is metric")
    ("non_metric",  po::value(&non_metric)->zero_tokens(),
                                                    "assume graph is non metric")
    ("symmetric",   po::value(&symmetric)->zero_tokens(),
                                                    "assume graph is symmetric")
    ("non_symmetric",
                    po::value(&non_symmetric)->zero_tokens(),
                                                    "assume graph is non symmetric")
    ("verbose,v",   po::value<int>(&verbose_level)->default_value(1),
                                                    "verbose level")
    ("print_smt",   po::value(&print_smt)->zero_tokens(),
                                                    "print smt2 instance")
    ("cnf2smt",     po::value(&cnf2smt)->zero_tokens(),
                                                    "convert cnf 2 smt2")
    ("output,o",    po::value<string>(&output_filename)->default_value(""),
                                                    "output filename")
    ("help,h",                                      "produce help message")
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
    
    // default theory
    if (!edge_theory && !lkh_theory && !mst_theory) {
        if (metric)
            lkh_theory = true;
        else
            edge_theory = true;
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
    if (metric) problem.assume_metric();
    if (non_metric) problem.assume_non_metric();
    if (symmetric) problem.assume_symmetric();
    if (non_symmetric) problem.assume_non_symmetric();
    if (edge_theory) problem.enable_edge_theory();
    if (lkh_theory) {
    if (!problem.enable_lkh_theory()) {
        printf("Error: could not enable LKH theory\n");
        return 0;
    }
    }
    if (mst_theory) {
        if (!problem.enable_mst_theory()) {
            printf("Error: could not enable MST theory\n");
            return 0;
        }
    }
    if (linear_search)
        problem.enable_linear_search();
    problem.set_bin_search_divider (bin_search_divider);
    problem.setConfBudget     (conflict_budget);
    problem.setPropBudget     (propagation_budget);
    if (max_usat_time > 0)
        problem.setUsatTimeBudget(max_usat_time);
    if (max_time > 0)
        problem.setTimeBudget(max_time);

    // print instance instead of solving instance
    if (cnf2smt) {
        problem.print_cnf_smt2();
        return 0;  
    }

    // print instance instead of solving instance
    if (print_smt) {
        problem.print_smt2(max_cost);
        return 0;  
    }

    // solve
    if (verbose_level >= 1) {
        printf("solving...\n");
    }
    bool result(false);
    if (max_cost >= 0) {
        result = problem.solve(max_cost);
        if (result) {
            problem.get_solution();
            assert (problem.verify_soln());
        }
    } else {
        result = problem.solve_tsp_optimal(verbose_level);
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
