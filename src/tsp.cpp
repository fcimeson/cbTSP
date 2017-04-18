
/****************************************************************************//**
 ********************************************************************************
 * @file        tsp.cpp
 * @brief       
 * @author      Frank Imeson
 * @date        
 ********************************************************************************
 ********************************************************************************/

#include "tsp.hpp"

//#define DEBUGsattsp.soln
//#define DEBUG00


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
string get_token(string line) {
  int i = line.find(':')+1;
  int j = line.find_last_not_of(' ')+1;
  if (line[i] == ' ') {
    i += line.substr(i,j).find_first_not_of(' ');
  }
  return line.substr(i,j-i);
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int nint(double x) {
  return int(floor(x+0.5));
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void extract_deg_min(double x, double &degrees, double &minutes) {
  if (x > 0) {
    degrees = floor(x);
    minutes = x - degrees;
  } else {
    degrees = ceil(x);
    minutes = x - degrees;
  }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * prereqs:
 *            soln is rotated
 ****************************************************************/
void TSP::split_vids(const vector<int> &vids, vector<vector<int>> &subgraph_vids) {
    int j(0);
    subgraph_vids = vector<vector<int>>(subgraphs.size());
    for (int i=0; i<vids.size(); i++) {
        // if vids[i] not in subgraph[j]
        while (!std::binary_search(subgraphs[j].begin(), subgraphs[j].end(), vids[i])) {
            j = (j+1) % subgraphs.size();
        }
        subgraph_vids[j].push_back(vids[i]);
    }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * prereqs:
 *            soln is rotated
 ****************************************************************/
bool TSP::get_tour_cost(const vector<int> &tour, int &tsp_cost, int &max_subgraph_cost) {

    // setup
    tsp_cost = 0;
    max_subgraph_cost = 0;
    vector<vector<int>> sub_tours;
    split_vids(tour, sub_tours);
    
    // calculate costs
    for (int i=0; i<sub_tours.size(); i++) {
        int tour_cost(0), vid00, vid01;
        for (int j=0; j<sub_tours[i].size(); j++) {
          vid00 = sub_tours[i][j];
          vid01 = sub_tours[i][(j+1)%sub_tours[i].size()];
          tour_cost += edge_weight[vid00][vid01];
        }
        tsp_cost += tour_cost;
        max_subgraph_cost = max(tour_cost, max_subgraph_cost);
    }
    
    return true;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * prereqs:
 *            soln is rotated
 ****************************************************************/
bool TSP::feasible(const vector<int> &soln_tour, const int tsp_cost_budget, const int subgraph_cost_budget) {

    int tsp_cost, max_subgraph_cost;
    get_tour_cost(soln_tour, tsp_cost, max_subgraph_cost);

    #ifdef DEBUG
        cout << "TSP::feasible():\n";
        cout << "  tour = ";
        for (int i=0; i<soln_tour.size(); i++) {
            cout << " " << soln_tour[i];
        }
        cout << '\n';
        cout << "  cost = " << tsp_cost << '\n';
    #endif
    return (tsp_cost <= tsp_cost_budget && max_subgraph_cost <= subgraph_cost_budget);
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool TSP::LKH(const vector<int> &vids, const int &max_cost, vector<int> &soln_tour, int &soln_cost) {

  vector<int> sorted_vids;
  for (unsigned int i=0; i<vids.size(); i++) {
    sorted_vids.push_back(vids[i]);
  }
  sort(sorted_vids.begin(), sorted_vids.end());

  #ifdef DEBUG
      cout << "sorted_vids:";
      for (int i=0; i < sorted_vids.size(); i++) {
          cout << " " << sorted_vids[i];
      }
      cout << '\n';
      cout << "  size = " << vids.size() << '\n';
      cout << "max_cost: " << max_cost << '\n';
  #endif

  if (vids.size() == 0) {
    return true;
  } else if (vids.size() == 1) {
    soln_cost = 0;
    soln_tour.clear();
    soln_tour.push_back(vids[0]);
    return true;
  } else if (vids.size() == 2) {
    int x(vids[0]), y(vids[1]);
    soln_cost = edge_weight[x][y] + edge_weight[y][x];
    if (soln_cost <= max_cost) {
      soln_tour.clear();
      soln_tour.push_back(x);
      soln_tour.push_back(y);
      return true;
    } else {
      return false;
    }
  } else if (vids.size() == 3) {
    int x(vids[0]), y(vids[1]), z(vids[2]);
    int cost01 = edge_weight[x][y] + edge_weight[y][z] + edge_weight[z][x];
    int cost02 = edge_weight[x][z] + edge_weight[z][y] + edge_weight[y][x];
    soln_cost = min(cost01, cost02);
    if (soln_cost <= max_cost) {
      soln_tour.clear();
      soln_tour.push_back(x);
      if (cost01 < cost02) {
        soln_tour.push_back(y);
        soln_tour.push_back(z);
      } else {
        soln_tour.push_back(z);
        soln_tour.push_back(y);
      }
      return true;
    } else {
      return false;
    }
  } else {

    // generate temporary filenames
    string tmp_dir;
    if (getenv("TEMP")) tmp_dir = getenv("TEMP");
    else                tmp_dir = "/tmp";
    string parm_str_filename = tmp_dir + "/sattsp.par.XXXXXX";
    string prob_str_filename = tmp_dir + "/sattsp.tsp.XXXXXX";
    string soln_str_filename = tmp_dir + "/sattsp.soln.XXXXXX";
    char* parm_filename = new char[parm_str_filename.size() + 1];
    char* prob_filename = new char[prob_str_filename.size() + 1];
    char* soln_filename = new char[soln_str_filename.size() + 1];
    std::copy(parm_str_filename.begin(), parm_str_filename.end(), parm_filename);
    std::copy(prob_str_filename.begin(), prob_str_filename.end(), prob_filename);
    std::copy(soln_str_filename.begin(), soln_str_filename.end(), soln_filename);
    parm_filename[parm_str_filename.size()] = '\0';
    prob_filename[prob_str_filename.size()] = '\0';
    soln_filename[soln_str_filename.size()] = '\0';
    int parm_fd = mkstemp(parm_filename);
    int prob_fd = mkstemp(prob_filename);
    int soln_fd = mkstemp(soln_filename);
    close (soln_fd);
    close (parm_fd);
    close (prob_fd);

    // parameter file
    ofstream parm_file;
    parm_file.open(parm_filename, ios::trunc);
    parm_file << "PROBLEM_FILE      = " << prob_filename << '\n';
    parm_file << "TOUR_FILE         = " << soln_filename << '\n';
    parm_file << get_lkh_parameters();
    parm_file << "TIME_LIMIT        = 10\n";
    parm_file << "STOP_AT_MAX_COST  = YES\n";
    parm_file << "MAX_COST          = " << int(max_cost) << '\n';
    parm_file << "SEED              = " << int(rand()) << '\n';
    parm_file.close();

    // problem file
    ofstream prob_file;
    prob_file.open(prob_filename, ios::trunc);
    prob_file << "NAME:                solver03\n";
    prob_file << "COMMENT:             solver03\n";
    prob_file << "TYPE:                ATSP\n";
    prob_file << "DIMENSION:           " << vids.size() << '\n';
    prob_file << "EDGE_WEIGHT_TYPE:    EXPLICIT\n";
    prob_file << "EDGE_WEIGHT_FORMAT:  FULL_MATRIX\n";
    prob_file << '\n';
    prob_file << "EDGE_WEIGHT_SECTION\n";
    for (unsigned int i=0; i<sorted_vids.size(); i++) {
      for (unsigned int j=0; j<sorted_vids.size(); j++) {
        int vid00 = sorted_vids[i];
        int vid01 = sorted_vids[j];
        prob_file << int(edge_weight[vid00][vid01]) << " ";
      }
      prob_file << '\n';
    }
    prob_file << "EOF\n";
    prob_file.close();

    // run LKH_cost
    string cmd = "cbLKH " + string(parm_filename);

    FILE* pipe = popen(cmd.c_str(), "r");
    char buffer[128];
    while (!feof(pipe)) {
      if(fgets(buffer, 128, pipe) != NULL) {
        string line(buffer);
        boost::regex soln_rx ("Cost.min = \\d+");
        if ( regex_search(line, soln_rx) ) {
          boost::regex int_rx("\\d+");
          boost::sregex_token_iterator token_itr (line.begin(), line.end(), int_rx, 0);
          boost::sregex_token_iterator end;
          soln_cost = atoi(string(*token_itr).c_str());
        }
		  }
    }
    pclose(pipe);

    // get tour
    if (soln_cost <= max_cost) {
      soln_tour.clear();
      ifstream soln_file(soln_filename);
      boost::regex int_rx("\\d+");

      string line;
      bool tour_section(false);
      while (getline(soln_file, line)) {

        if (line == "TOUR_SECTION")
          tour_section = true;
        
        if (tour_section && regex_match(line, int_rx) )
          soln_tour.push_back(sorted_vids[atoi(line.c_str())-1]);

        if (tour_section && line == "-1\n")
          tour_section = false;
      }
      soln_file.close();
      remove(soln_filename);
    }

    remove(parm_filename);
    remove(prob_filename);
    remove(soln_filename);
    delete[] parm_filename;
    delete[] prob_filename;
    delete[] soln_filename;
  }
  return (soln_cost <= max_cost);
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool TSP::solve(const vector<int> &vids, const int tsp_cost_budget, const int subgraph_cost_budget, vector<int> &soln_tour, int &soln_cost) {

  // seperate vids into subgraph_vids
  int i(0), j(0), x, y, z;
  vector<vector<int>> subgraph_vids(subgraphs.size());
  for (int i=0; i<vids.size(); i++) {
      // if vids[i] not in subgraph[j]
      while (!std::binary_search(subgraphs[j].begin(), subgraphs[j].end(), vids[i])) {
          j = (j+1) % subgraphs.size();
      }
      subgraph_vids[j].push_back(vids[i]);
  }

  #ifdef DEBUG
      cout << "TSP::solve():\n";
      for (int i=0; i < subgraph_vids.size(); i++) {
          cout << "  subgraph_vids[" << i << "]:";
          for (int j=0; j < subgraph_vids[i].size(); j++) {
              cout << " " << subgraph_vids[i][j];
          }
          cout << '\n';
      }
      cout << "  tsp_cost_budget = " << tsp_cost_budget << '\n';
      cout << "  subgraph_cost_budget = " << subgraph_cost_budget << '\n';
  #endif

  // solve each subgraph
  int tsp_cost(0);
  vector<int> subgraph_cost(subgraphs.size());
  vector<vector<int>> subgraph_tour(subgraphs.size());
  for (int i=0; i<subgraphs.size(); i++) {
      if (LKH(subgraph_vids[i], subgraph_cost_budget, subgraph_tour[i], subgraph_cost[i])) {
          tsp_cost += subgraph_cost[i];
      } else {
          return false;
      }
  }
  
  #ifdef DEBUG
      for (int i=0; i < subgraphs.size(); i++) {
          cout << "initial_subgraph_solns[" << i << "]: \n";
          cout << "  cost = " << subgraph_cost[i] << '\n';
          cout << "  tour =";
          for (int j=0; j < subgraph_tour[i].size(); j++) {
              cout << " " << subgraph_tour[i][j];
          }
          cout << '\n';
      }
  #endif

  // re-solve until within tsp_cost_budget budget
  i = j = 0;
  vector<bool> subgraph_minimized(subgraphs.size());
  for (int i=0; i<subgraphs.size(); i++) {
      if (subgraph_vids[i].size() <= 3) {
          subgraph_minimized[i] = true;
      } else {
          subgraph_minimized[i] = false;
      }
  }
  while (tsp_cost > tsp_cost_budget) {
      if (!subgraph_minimized[i]) {
          int cost;
          vector<int> tour;
          int test_min, test_cost, test_max(subgraph_cost[i]-1);
          test_min = subgraph_cost[i] - (tsp_cost-tsp_cost_budget);
          test_cost = test_min + floor((test_max-test_min)/2);
          while(true) {
              if (LKH(subgraph_vids[i], test_cost, tour, cost)) {
                  tsp_cost -= subgraph_cost[i];
                  subgraph_cost[i] = cost;
                  subgraph_tour[i] = tour;
                  tsp_cost += subgraph_cost[i];
                  j = i; // keep track of last tsp solved
                  #ifdef DEBUG
                      cout << "  solved:\n";
                      cout << "    cost = " << cost << '\n';
                      cout << "    test_cost = " << test_cost << '\n';
                      if (feasible(tour, test_cost, test_cost)) {
                          cout << "    feasible\n";
                      } else {
                          cout << "    not feasible\n";
                      }
                  #endif
                  break;
              } else {
                  test_min = test_cost+1;
                  test_cost = test_min + floor((test_max-test_min)/2);
                  if (test_min > test_max) {
                      subgraph_minimized[i] = true;
                      break;
                  }
              }
              #ifdef DEBUG
                  cout << "  test_min  = " << test_min << '\n';
                  cout << "  test_cost = " << test_cost << '\n';
                  cout << "  test_max  = " << test_max << '\n';
              #endif
          }
      } else {
          i = (i+1) % subgraphs.size();
          if (i == j) {
              return false; // if we get back to the last one solved and still no soln, then quit
          }
      }
  }
  
  // concatenate solution
  if (type == "MIN_MAX_TSP") {
      int max_cost(0);
      for (int i=0; i<subgraphs.size(); i++) {
          max_cost = max(subgraph_cost[i], max_cost);
      }
      soln_cost = max_cost;
  } else {
      soln_cost = tsp_cost;
  }
  soln_tour.clear();
  for (int i=0; i<subgraphs.size(); i++) {
      soln_tour.insert(soln_tour.end(), subgraph_tour[i].begin(), subgraph_tour[i].end());
  }
  
  #ifdef DEBUG
      for (int i=0; i < subgraphs.size(); i++) {
          cout << "finial_subgraph_solns[" << i << "]: \n";
          cout << "  cost = " << subgraph_cost[i] << '\n';
          cout << "  tour =";
          for (int j=0; j < subgraph_tour[i].size(); j++) {
              cout << " " << subgraph_tour[i][j];
          }
          cout << '\n';
      }
      cout << "tsp cost = " << soln_cost << '\n';
  #endif

  return true;
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int parse_input(string filename, TSP &graph) {

  // reset graph
  // TODO: implement reset() method for tsp class
  graph.adjacency.clear();
  graph.edge_weight.clear();

  // open file
  ifstream file;
  try {
    file.open(filename.c_str());
  } catch(exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  } catch(...) {
    cerr << "Exception of unknown type!\n";
  }
  
  
  // helper structures
  string parse_state = "HEADER";
  string data_type = "";
  int vid00(0), vid01(0), subgraph_index(0), gtsp_set_index(0);
  vector<vector<double>> coords;
  while (file.good()) {
    string line;
    getline(file, line);
//    cout << line << '\n';

    // setup
    if (parse_state == "HEADER") {
    
      // name
      boost::regex name_rx ("^\\s*NAME\\s*:?\\s*[\\w_]+\\s*$");
      if ( regex_match(line, name_rx) ) {
        graph.name = get_token(line);
      }

      // type
      boost::regex problem_type_rx ("^\\s*TYPE\\s*:?\\s*[\\w_]+\\s*$");
      if ( regex_match(line, problem_type_rx) ) {
        graph.type = get_token(line);
      }

      // dimension
      boost::regex size_rx ("^\\s*DIMENSION\\s*:?\\s*\\d+\\s*$");
      if ( regex_match(line, size_rx) ) {
        int size = atoi(get_token(line).c_str());
        for (int i=0; i<size; i++) {
          graph.adjacency.push_back(vector<bool>());
          graph.edge_weight.push_back(vector<int>());
          for (int j=0; j<size; j++) {
            graph.adjacency[i].push_back(false);
            graph.edge_weight[i].push_back(-1);
          }
        }
      }

      // max tsp cost
      boost::regex tsp_cost_budget_rx ("^\\s*MAX_TSP_COST\\s*:?\\s*\\d+\\s*$");
      if ( regex_match(line, tsp_cost_budget_rx) ) {
        int cost = atoi(get_token(line).c_str());
        graph.tsp_cost_budget = cost;
      }

      // max subgraph cost
      boost::regex subgraph_cost_budget_rx ("^\\s*MAX_SUBGRAPH_COST\\s*:?\\s*\\d+\\s*$");
      if ( regex_match(line, subgraph_cost_budget_rx) ) {
        int cost = atoi(get_token(line).c_str());
        graph.subgraph_cost_budget = cost;
      }
      
      // edge weight type
      boost::regex edge_weight_type_rx ("^\\s*EDGE_WEIGHT_TYPE\\s*:?\\s*[\\w_]+\\s*$");
      if ( regex_match(line, edge_weight_type_rx) ) {
        data_type = get_token(line);
      }
      
      // edge weight format
      boost::regex edge_weight_format_rx ("^\\s*EDGE_WEIGHT_FORMAT\\s*:?\\s*[\\w_]+\\s*$");
      if (regex_search(line, edge_weight_format_rx) && data_type == "EXPLICIT") {
        data_type = get_token(line);
      }

    // parse matrix
    } else if (parse_state == "MATRIX_DATA") {
    
      boost::regex matrix_row_rx ("^\\s*\\d+");
      if ( regex_search(line, matrix_row_rx) ) {
        boost::regex int_rx("\\d+");
        boost::sregex_token_iterator token_itr (line.begin(), line.end(), int_rx, 0);
        boost::sregex_token_iterator end;
        while (token_itr != end) {
          int weight = atoi(string(*token_itr).c_str());
          token_itr++;
          // tested: 16.12.02
          if (data_type == "FULL_MATRIX") {
            graph.adjacency[vid00][vid01]   = (weight < INF);
            graph.edge_weight[vid00][vid01] = weight;
            vid01++;
            if (vid01 >= graph.size()) {
              vid00++;
              vid01 = 0;
            }
          // not tested:
          } else if (data_type == "LOWER_ROW") {
            cerr << "WARNING: LOWER_ROW has not been tested! \n";
            if (vid00 == 0 && vid01 == 0) {
                vid00 = 1;
            }
            graph.adjacency[vid00][vid01]   = (weight < INF);
            graph.adjacency[vid01][vid00]   = (weight < INF);
            graph.edge_weight[vid00][vid01] = weight;
            graph.edge_weight[vid01][vid00] = weight;
            vid01++;
            if (vid01 >= vid00) {
              vid00++;
              vid01 = 0;
            }
          // tested: 16.12.02
          } else if (data_type == "LOWER_DIAG_ROW") {
            graph.adjacency[vid00][vid01]   = (weight < INF);
            graph.adjacency[vid01][vid00]   = (weight < INF);
            graph.edge_weight[vid00][vid01] = weight;
            graph.edge_weight[vid01][vid00] = weight;
            vid01++;
            if (vid01 > vid00) {
              vid00++;
              vid01 = 0;
            }
          // tested: 16.12.02
          } else if (data_type == "UPPER_ROW") {
            if (vid00 == 0 && vid01 == 0) {
                vid01 = 1;
            }
            graph.adjacency[vid00][vid01]   = (weight < INF);
            graph.adjacency[vid01][vid00]   = (weight < INF);
            graph.edge_weight[vid00][vid01] = weight;
            graph.edge_weight[vid01][vid00] = weight;
            vid01++;
            if (vid01 >= graph.size()) {
              vid00++;
              vid01 = vid00+1;
            }
          // not working: 16.12.02
          } else if (data_type == "UPPER_DIAG_ROW") {
            cerr << "WARNING: UPPER_DIAG_ROW has not been tested! \n";
            graph.adjacency[vid00][vid01]   = (weight < INF);
            graph.adjacency[vid01][vid00]   = (weight < INF);
            graph.edge_weight[vid00][vid01] = weight;
            graph.edge_weight[vid01][vid00] = weight;
            vid01++;            
            if (vid01 >= graph.size()) {
              vid00++;
              vid01 = vid00;
            }
          } else {
            cerr << "error: invalid parsing state\n";
            return 1;
          }
        }
      }
    
    // parse coordinates
    } else if (parse_state == "COORD_DATA") {
      boost::regex float_coordinate_rx ("^\\s*\\d+\\s+-?\\d+\\.\\d+\\s+-?\\d+\\.\\d+");
      boost::regex int_coordinate_rx ("^\\s*\\d+\\s+-?\\d+\\s+-?\\d+");
      vector<double> coord;
      
      if ( regex_search(line, float_coordinate_rx) ) {
        boost::regex double_rx("-?\\d+\\.\\d+");
        boost::sregex_token_iterator token_itr (line.begin(), line.end(), double_rx, 0);
        coord.push_back(atof(string(*token_itr).c_str()));
        token_itr++;
        coord.push_back(atof(string(*token_itr).c_str()));
        coords.push_back(coord);
      } else if ( regex_search(line, int_coordinate_rx) ) {
        boost::regex int_rx("-?\\d+");
        boost::sregex_token_iterator token_itr (line.begin(), line.end(), int_rx, 0);
        token_itr++;
        coord.push_back(atoi(string(*token_itr).c_str()));
        token_itr++;
        coord.push_back(atoi(string(*token_itr).c_str()));
        coords.push_back(coord);
      }

    // parse subgraph data
    } else if (parse_state == "SUBGRAPH_DATA") {
      boost::regex data_rx ("^\\s*\\d+\\s+-?\\d+\\s+-?\\d+");
      if ( regex_search(line, data_rx) ) {
        int data(-1);
        boost::regex int_rx("-?\\d+");
        boost::sregex_token_iterator token_itr (line.begin(), line.end(), int_rx, 0);
        data = atoi(string(*token_itr).c_str());
        if (data == subgraph_index+1) {
          graph.subgraphs.push_back(vector<int>());
          token_itr++;
        }
        boost::sregex_token_iterator end;
        while (token_itr != end) {
          data = atoi(string(*token_itr).c_str());
          token_itr++;
          if (data == -1) {
            sort(graph.subgraphs[subgraph_index].begin(), graph.subgraphs[subgraph_index].end());
            subgraph_index++;
          } else {
            graph.subgraphs[subgraph_index].push_back(data-1);
          }
        }
      }

    // parse GTSP data
    } else if (parse_state == "GTSP_DATA") {
      boost::regex data_rx ("^\\s*\\d+\\s+-?\\d+\\s+-?\\d+");
      if ( regex_search(line, data_rx) ) {
        int data(-1);
        boost::regex int_rx("-?\\d+");
        boost::sregex_token_iterator token_itr (line.begin(), line.end(), int_rx, 0);
        data = atoi(string(*token_itr).c_str());
        if (data == gtsp_set_index+1) {
          graph.gtsp_sets.push_back(vector<int>());
          token_itr++;
        }
        boost::sregex_token_iterator end;
        while (token_itr != end) {
          data = atoi(string(*token_itr).c_str());
          token_itr++;
          if (data == -1) {
            gtsp_set_index++;
          } else {
            graph.gtsp_sets[gtsp_set_index].push_back(data-1);
          }
        }
      }
    }

    
    // transitions
    boost::regex edge_data_rx ("^\\s*EDGE_WEIGHT_SECTION\\s*:?\\s*$");
    if ( regex_match(line, edge_data_rx) ) {
      parse_state = "MATRIX_DATA";
    }
    boost::regex coord_data_rx ("^\\s*NODE_COORD_SECTION\\s*:?\\s*$");
    if ( regex_match(line, coord_data_rx) ) {
      parse_state = "COORD_DATA";
    }
    boost::regex subgraph_rx ("^\\s*SUBGRAPH_SECTION\\s*:?\\s*$");
    if ( regex_match(line, subgraph_rx) ) {
      parse_state = "SUBGRAPH_DATA";
    }
    boost::regex gtsp_rx ("^\\s*GTSP_SET_SECTION\\s*:?\\s*$");
    if ( regex_match(line, gtsp_rx) ) {
      parse_state = "GTSP_DATA";
    }
    boost::regex display_rx ("^\\s*DISPLAY_DATA_SECTION\\s*:?\\s*$");
    if ( regex_match(line, display_rx) ) {
      parse_state = "DISPLAY_DATA";
    }

  }

  // convert coord data to matrix
  // tested: 16.12.02
  if (data_type == "EUC_2D") {
    for (int vid00=0; vid00 < graph.size(); vid00++) {
      for (int vid01=0; vid01 < graph.size(); vid01++) {
        if (vid00 == vid01) {
          graph.adjacency[vid00][vid01]   = false;
          graph.edge_weight[vid00][vid01] = INF;
        } else {
          double dx = coords[vid00][0] - coords[vid01][0];
          double dy = coords[vid00][1] - coords[vid01][1];
          double weight = sqrt(pow(dx,2) + pow(dy,2));
          graph.adjacency[vid00][vid01]   = true;
          graph.edge_weight[vid00][vid01] = nint(weight);
        }        
      }
    }
  // tested: 16.12.02
  } else if (data_type == "GEO") {
    double RRR = 6378.388;
    double PI = 3.141592;                
    double deg(0), min(0);
    for (int vid00=0; vid00 < graph.size(); vid00++) {
      extract_deg_min(coords[vid00][0], deg, min);
      double lat00 = PI * (deg + 5.0 * min / 3.0) / 180.0;
      extract_deg_min(coords[vid00][1], deg, min);
      double long00 = PI * (deg + 5.0 * min / 3.0) / 180.0;     
      for (int vid01=0; vid01 < graph.size(); vid01++) {
        extract_deg_min(coords[vid01][0], deg, min);
        double lat01 = PI * (deg + 5.0 * min / 3.0) / 180.0;
        extract_deg_min(coords[vid01][1], deg, min);
        double long01 = PI * (deg + 5.0 * min / 3.0) / 180.0;     
        if (vid00 == vid01) {
          graph.adjacency[vid00][vid01]   = false;
          graph.edge_weight[vid00][vid01] = INF;
        } else {
          double q1 = cos(long00 - long01);
          double q2 = cos(lat00 - lat01);
          double q3 = cos(lat00 + lat01);
          double weight = RRR * acos(0.5*((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0;
          graph.adjacency[vid00][vid01]   = true;
          graph.edge_weight[vid00][vid01] = int(weight);

          #ifdef DEBUG96
            cout << "coord[" << vid00 << "] = (" << coords[vid00][0] << "," << coords[vid00][1] << "), ";
            cout << "coord[" << vid01 << "] = (" << coords[vid01][0] << "," << coords[vid01][1] << ")\n";
            cout << "lat00 = " << lat00 << ", long00 = " << long00 << ", lat01 = " << lat01 << ", long01 = " << long01 << '\n';
            cout << "q1 = " << q1 << ", q2 = " << q2 << ", q3 = " << q3 << ", weight = " << weight << '\n';
          #endif          
        }          
      }
    }
  // tested: 16.12.02
  } else if (data_type == "ATT") {
    for (int vid00=0; vid00 < graph.size(); vid00++) {
      for (int vid01=0; vid01 < graph.size(); vid01++) {
        if (vid00 == vid01) {
          graph.adjacency[vid00][vid01]   = false;
          graph.edge_weight[vid00][vid01] = INF;
        } else {
          double dx = coords[vid00][0] - coords[vid01][0];
          double dy = coords[vid00][1] - coords[vid01][1];
          double r = sqrt(dx*dx/10.0 + dy*dy/10.0);
          int weight = nint(r);
          if (weight < r) {
            weight++;
          }
          graph.adjacency[vid00][vid01]   = true;
          graph.edge_weight[vid00][vid01] = weight;
          #ifdef DEBUG96
            cout << "coord: " << coords[vid00][0] << "," << coords[vid00][1] << '\n';
            cout << "coord: " << coords[vid01][0] << "," << coords[vid01][1] << '\n';
            cout << "weight: " << weight << '\n';
          #endif          
        }
      }      
    }
  // not tested
  } else if (data_type == "CEIL_2D") {
    cerr << "WARNING: CEIL_2D has not been tested! \n";
    for (int vid00=0; vid00 < graph.size(); vid00++) {
      for (int vid01=0; vid01 < graph.size(); vid01++) {
        if (vid00 == vid01) {
          graph.adjacency[vid00][vid01]   = false;
          graph.edge_weight[vid00][vid01] = INF;
        } else {
          double dx = coords[vid00][0] - coords[vid01][0];
          double dy = coords[vid00][1] - coords[vid01][1];
          double weight = sqrt(pow(dx,2) + pow(dy,2));
          graph.adjacency[vid00][vid01]   = true;
          graph.edge_weight[vid00][vid01] = ceil(weight);
        }
      }
    }
  }
  
  // setup default subgraph
  if (graph.subgraphs.size() == 0) {
    graph.subgraphs.push_back(vector<int>());
    for (int vid00=0; vid00 < graph.size(); vid00++) {
      graph.subgraphs[0].push_back(vid00);
    }
  }
  
  #ifdef OLD91
  // find min and max edge weights
  graph.min_edge_cost = 99999999;
  graph.max_edge_cost = 0;
  for (int i=0; i < graph.size(); i++) {
    for (int j=0; j < graph.size(); j++) {
      graph.edge_weight[i][j] = round(graph.edge_weight[i][j]);
      if (graph.edge_weight[i][j] > graph.max_edge_cost)
        graph.max_edge_cost = graph.edge_weight[i][j];
      if (graph.edge_weight[i][j] < graph.min_edge_cost)
        graph.min_edge_cost = graph.edge_weight[i][j];
    }
  }
  #endif

  #ifdef DEBUG
    printf("nVertices=%d\n", graph.size());
    cout << "Edge Weight:\n";
    for (int i=0; i < graph.size(); i++) {
      for (int j=0; j < graph.size(); j++) {
        if (graph.adjacency[i][j])
          printf("%7d", int(graph.edge_weight[i][j]));
        else
          printf("%7d", 999999);
      }  
      printf("\n");
    }
    printf("nSubgraphs=%d\n", graph.subgraphs.size());
    for (int i=0; i < graph.subgraphs.size(); i++) {
      printf("Sugraph[%d] =", i);
      for (int j=0; j < graph.subgraphs[i].size(); j++) {
        printf(" %d", graph.subgraphs[i][j]);
      }
      printf("\n");
    }
  #endif

}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::tsp2cnf (Solver* solver) {

    // reduce tsp to sat
    formula::Formula* tsp_formula = new formula::Formula(formula::F_AND);

    // setup structures
    theory_vars.clear();
    for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
        vid_incoming_vars.push_back(vector<int>());
        vid_outgoing_vars.push_back(vector<int>());
    }

    // constrain vertices to be in the graph if the edge is in the graph
    edge_var_offset               = -1;
    Lit edge_lits[adjacency.size()][adjacency.size()];
    for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
        for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++) {
            Lit edge_lit                  = mkLit(solver->newVar());
            Lit source_lit                = mkLit(vid00);
            Lit target_lit                = mkLit(vid01);
            edge_lits[vid00][vid01]       = edge_lit;
            if (edge_var_offset < 0)
                edge_var_offset = var(edge_lit);
            // edge implies source and target
            tsp_formula->add(formula::constraint_implies(edge_lit, source_lit));
            tsp_formula->add(formula::constraint_implies(edge_lit, target_lit));
        }
    }

    #ifdef VER100
        // constrain vertices to have only one incoming and one outgoing edge
        for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
            vector<int> edge_vars;
            formula::Formula *f0, *f1;

            // incoming edges
            edge_vars.clear();
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++)
                if (adjacency[vid01][vid00])
                    edge_vars.push_back(var(edge_lits[vid01][vid00]));
            f0 = formula::constraint_one_in_a_set(edge_vars);
            f1 = formula::constraint_implies(mkLit(vid2var(vid00)),f0);
            tsp_formula->add(f1);

            // outgoing edges
            edge_vars.clear();
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++)
                if (adjacency[vid00][vid01])
                    edge_vars.push_back(var(edge_lits[vid00][vid01]));
            f0 = formula::constraint_one_in_a_set(edge_vars);
            f1 = formula::constraint_implies(mkLit(vid2var(vid00)),f0);
            tsp_formula->add(f1);
        }

        // constrain solution to only have one cycle (every vertex is reachable from every vertex)

        // create reachable variables
        // reachable[level][vid01] => vid01 is reachable on level
        Lit reachable[adjacency.size()+1][adjacency.size()];
        for (unsigned int level=0; level < adjacency.size()+1; level++)
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++)
                reachable[level][vid01] = mkLit(solver->newVar());

        // level 0 of the soln has exactly one vertex true
        // vid00 is reachable on level 0 => vid00 is in the solution
        vector<int> vids;
        for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
            vids.push_back(var(reachable[0][vid00]));
            tsp_formula->add(formula::constraint_implies(reachable[0][vid00], mkLit(vid2var(vid00))));
        }
        tsp_formula->add(formula::constraint_one_in_a_set(vids));
    #else
        // constrain vertices to have only one incoming edge
        for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
            formula::Formula *f0 = new formula::Formula(formula::F_OR);
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++) {
                if (adjacency[vid01][vid00]) {
                    int edge_var = var(edge_lits[vid01][vid00]);
                    vid_incoming_vars[vid00].push_back(edge_var);
                    theory_vars.push_back(edge_var);
                    Lit edge_lit = mkLit(edge_var, false);
                    f0->add(edge_lit);
                }
            }
            formula::Formula *f1 = formula::constraint_implies(mkLit(vid2var(vid00)),f0);
            tsp_formula->add(f1);
        }

        // constrain vertices to have only one outgoing edge
        for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
            formula::Formula *f0 = new formula::Formula(formula::F_OR);
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++) {
                if (adjacency[vid00][vid01]) {
                    int edge_var = var(edge_lits[vid00][vid01]);
                    vid_outgoing_vars[vid00].push_back(edge_var);
                    theory_vars.push_back(edge_var);
                    Lit edge_lit = mkLit(edge_var, false);
                    f0->add(edge_lit);
                }
            }
            formula::Formula *f1 = formula::constraint_implies(mkLit(vid2var(vid00)),f0);
            tsp_formula->add(f1);
        }

        /*
         * constrain solution to only have one cycle (every vertex is reachable from every vertex)
         */

        // reachable[level][vid01] => vid01 is reachable on level
        Lit reachable[adjacency.size()+1][adjacency.size()];
        for (unsigned int level=0; level < adjacency.size()+1; level++)
            for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++)
                reachable[level][vid01] = mkLit(solver->newVar());

        // vid00 is reachable on level 0 => vid00 is in the solution
        level00_vars.clear();
        for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
            level00_vars.push_back(var(reachable[0][vid00]));
            tsp_formula->add(formula::constraint_implies(reachable[0][vid00], mkLit(vid2var(vid00))));
        }
        // level 0 of the soln has exactly one vertex true
        for (unsigned int i=0; i < level00_vars.size(); i++) {
            int vid_var = level00_vars[i];
            theory_vars.push_back(vid_var);
        }
        tsp_formula->add(formula::constraint_at_least_one_in_a_set(level00_vars));
    #endif

    for (unsigned int level=1; level < adjacency.size()+1; level++) {
        for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++) {
            formula::Formula *f0 = new formula::Formula(formula::F_OR);
            for (unsigned int vid00=0; vid00 < adjacency.size(); vid00++) {
                // vid01 is reachable
                if (vid00 != vid01 && adjacency[vid00][vid01]) {
                    formula::Formula *f1 = new formula::Formula(formula::F_AND);
                    f1->add(reachable[level-1][vid00]);
                    f1->add(edge_lits[vid00][vid01]);
                    f0->add(f1);
                }
            }
            // vid01 is reachable iff it is reachable from a reachable source
            Lit f0_lit;
            f0->export_cnf(f0_lit, NULL, solver);
            tsp_formula->add(formula::constraint_iff(f0_lit, reachable[level][vid01]));
        }
    }

    // if vertex is in the solution then it is reachable
    // FCI: change to iff and only if (remove reachable on a level line 433
    for (unsigned int vid01=0; vid01 < adjacency.size(); vid01++) {
        formula::Formula *f0 = new formula::Formula(formula::F_OR);
        for (unsigned int level=1; level < adjacency.size()+1; level++) {
            f0->add(reachable[level][vid01]);
        }
        Lit f0_lit;
        f0->export_cnf(f0_lit, NULL, solver);
        tsp_formula->add(formula::constraint_implies(mkLit(vid2var(vid01)), f0_lit));
    }

    Lit cnf_out;
    if (int err = tsp_formula->export_cnf(cnf_out, NULL, solver) < 0) {
        #ifdef DEBUG
            cerr << "Error in Setup: export_cnf failed" << endl;
        #endif
    }
    solver->addClause(cnf_out);
    #ifdef DEBUG_SAT
        cout << "tsp formula: " << endl << tsp_formula->str();
    #endif  

    return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::vid2var (const int vid)
{
  return vid;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::var2vid (const int var)
{
  if (var < adjacency.size())
    return var;
  else
    return -1;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::eid2var (const int eid)
{
  return eid + edge_var_offset;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::var2eid (const int var)
{
  int eid = var - edge_var_offset;
  if (eid >= 0 && eid < adjacency.size()*adjacency.size())
    return eid;
  else
    return -1;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::lit_cost (const Lit &lit)
{
  int eid = var2eid(var(lit));
  if (eid >= 0 && sign(lit) == false) {
    int i,j;
    eid2edge(eid, i, j);
    return edge_weight[i][j];
  } else return 0;
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
void TSP::eid2edge (const int eid, int &from, int &to)
{
  if (eid >= 0 && eid < adjacency.size()*adjacency.size()) {
    from  = floor(eid/adjacency.size());
    to    = eid % adjacency.size();
  } else {
    from  = -1;
    to    = -1;
 }
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::edge2eid (const int from, const int to)
{
  if (from < adjacency.size() && to < adjacency.size())
    return adjacency.size()*from + to;
  return -1;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
int TSP::eid_cost (int eid)
{
  if (eid < 0 || eid > adjacency.size()*adjacency.size() )
    return 999999999;

  int from, to;
  eid2edge(eid, from, to);
  return edge_weight[from][to];
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool TSP::metric ()
{
  if (_metric == 1) {
    return true;
  } else if (_metric == 0) {
    return false;
  } else {
    for (int vid00=0; vid00<this->size(); vid00++) {
      for (int vid01=0; vid01<this->size(); vid01++) {
        for (int vid02=0; vid02<this->size(); vid02++) {
          if (vid00 != vid01 && vid00 != vid02 && vid01 != vid02) {
            if (edge_weight[vid00][vid01] + edge_weight[vid01][vid02] - edge_weight[vid00][vid02] < -1) {
              _metric = 0;
              return false;
            }
          }
        }
      }  
    }
    _metric = 1;
    return true;
  }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool TSP::symmetric ()
{
  if (_symmetric == 1) {
    return true;
  } else if (_symmetric == 0) {
    return false;
  } else {
    for (int vid00=0; vid00<this->size(); vid00++) {
      for (int vid01=0; vid01<this->size(); vid01++) {
        if ( abs(edge_weight[vid00][vid01] - edge_weight[vid01][vid00]) > 1 ) {
          _symmetric = 0;
          return false;
        }
      }  
    }
    _symmetric = 1;
    return true;
  }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
bool TSP::tsp_monotonic ()
{
  if (_tsp_monotonic == 1) {
    return true;
  } else if (_tsp_monotonic == 0) {
    return false;
  } else {
    if (metric()) {
        _tsp_monotonic = 1;
        return true;
    } else {
        return false;
    }
  }
}



/************************************************************//**
 * @brief	
 * @version						v0.01b
 * Algorithm:
 *  http://www.sciencedirect.com/science/article/pii/0022000078900223
 ****************************************************************/
void TSP::set_lkh_parameters (string filename)
{
  ifstream file;
  try {
    file.open(filename.c_str());
  } catch(exception& e) {
    cerr << "error: " << e.what() << "\n";
  } catch(...) {
    cerr << "Exception of unknown type!\n";
  }
  
  lkh_parameters = "";
  while (file.good()) {
    string line;
    getline(file, line);
    lkh_parameters += line + '\n';
  }
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * Algorithm:
 *  http://www.sciencedirect.com/science/article/pii/0022000078900223
 ****************************************************************/
string TSP::get_lkh_parameters ()
{
  return lkh_parameters;
}




/************************************************************//**
 * @brief	
 * @version						v0.01b
 ****************************************************************/
MST::MST (TSP* _graph)
  : _size(0)
  , _cost(0)
  , subset(_graph->size(), false)
  , adjacency(_graph->size(), vector<bool>(_graph->size(), false))
{
  graph = _graph;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * Prim's Algorithm:  http://en.wikipedia.org/wiki/Prim%27s_algorithm
 ****************************************************************/
MST::MST (TSP* graph, const vector<int> subset)
  : _size(0)
  , _cost(0)
  , subset(graph->size(), false)
  , adjacency(graph->size(), vector<bool>(graph->size(), false))
{
  this->graph = graph;
  vector<int> eids, Va, Vb(subset);
  vector<bool> VaBool(graph->size(), false), VbBool(graph->size(), false);

  for (int i=0; i<Vb.size(); i++)
    VbBool[Vb[i]] = true;

  int va = Vb.back();
  Vb.pop_back();
  Va.push_back(va);
  VaBool[va] = true;
  VbBool[va] = false;
  for (int i=0; i<Vb.size(); i++) {
    eids.push_back(graph->edge2eid(va, Vb[i]));
//    eids.push_back(graph->edge2eid(Vb[i], va));
  }

  #ifdef DEBUG
    printf("MST::MST_PRIMS()\n");
    for (int j=0; j<subset.size(); j++)
      printf("  |- subset[%d] = %d\n", j, subset[j]);
    for (int j=0; j<Va.size(); j++)
      printf("  |- Va[%d] = %d\n", j, Va[j]);
    for (int j=0; j<Vb.size(); j++)
      printf("  |- Vb[%d] = %d\n", j, Vb[j]);
    for (int j=0; j<eids.size(); j++) {
      int vid00, vid01;
      graph->eid2edge(eids[j], vid00, vid01);      
      printf("  |- eids[%d] = <%d,%d>, cost = %f\n", j, vid00, vid01, graph->edge_weight[vid00][vid01]);
    }
  #endif

  while (Vb.size() > 0) {
    sort(eids.begin(), eids.end(), *graph);
    for (int i=eids.size()-1; i>=0; i--) {
      int eid = eids[i];
      int from, to;
      graph->eid2edge(eid, from, to);
      #ifdef DEBUG
        printf("  |- canidate edge <%d,%d>\n", from, to);
      #endif
      if (VaBool[from] && VbBool[to]) {
        #ifdef DEBUG
          printf("  |- minimum edge\n");
          for (int j=0; j<Va.size(); j++)
            printf("    |- Va[%d] = %d\n", j, Va[j]);
          for (int j=0; j<Vb.size(); j++)
            printf("    |- Vb[%d] = %d\n", j, Vb[j]);
        #endif
        int vb_index(-1);
        for (int j=0; j<Vb.size(); j++) {
          if (Vb[j] == to) {
            vb_index = j;
          } else {
            // add edges now reachable from to
            eids.push_back(graph->edge2eid(to, Vb[j]));
//            eids.push_back(graph->edge2eid(Vb[j], to));
            #ifdef DEBUG
              printf("  |- prims new edge <%d,%d>.cost() = %d\n", to, Vb[j], int(graph->edge_weight[to][Vb[j]]));
            #endif
          }
        }
        #ifdef DEBUG
          printf("  |- prims min edge <%d,%d>.cost() = %d\n", from, to, int(graph->edge_weight[from][to]));
        #endif

        Va.push_back(to);
        Vb.erase(Vb.begin()+vb_index);
        VaBool[to]          = true;
        VbBool[to]          = false;
        adjacency[from][to] = true;
        adjacency[to][from] = true;
        _cost += graph->edge_weight[from][to];
        eids.erase(eids.begin()+i);
        break;
      }
    }
  }

  this->subset = VaBool;
  _size  = Va.size();
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 * Algorithm:
 *  http://www.sciencedirect.com/science/article/pii/0022000078900223
 ****************************************************************/
int MST::adjacent (int vid00, vector<int> &list)
{
  list.clear();
  for (int vid01=0; vid01<graph->size(); vid01++) {
    if (adjacency[vid00][vid01])
      list.push_back(vid01);
  }
  return 0;
}


/************************************************************//**
 * @brief	
 * @version						v0.01b
 *
 * @param[in]
 *  z_vid             new vertex to add to tree
 *  r_vid             current vertex insertion into new tree
 *  t_eid             largest edge in path between w_vid and z_vid
 *  inserted          edges already inserted into new tree
 *  new_adjacency     edges in new tree
 *
 * @param[out]
 *  new_adjacency     edges in MST
 *  new_adjacency     edges in MST
 *
 * Algorithm:         http://www.sciencedirect.com/science/article/pii/0022000078900223
 *  w_vid             
 *  m_eid             largest edge between r_vid and z_vid
 ****************************************************************/
void MST::_insert (int r_vid, int &t_eid)
{
  new_subset[r_vid] = true;
  int m_eid         = graph->edge2eid(r_vid, z_vid);

  vector<int> adjacent_list;
  this->adjacent(r_vid, adjacent_list);
  for (int i=0; i<adjacent_list.size(); i++) {
    int w_vid = adjacent_list[i];
    int w_eid = graph->edge2eid(w_vid, r_vid);

    // adjacent vertices from old MST
    if (!new_subset[w_vid] && graph->adjacency[w_vid][r_vid]) {

      this->_insert(w_vid, t_eid);
      int k_eid, h_eid;
      
      if (graph->edge_weight[w_vid][r_vid] > graph->eid_cost(t_eid)) {
        k_eid = w_eid;
        h_eid = t_eid;
      } else {
        k_eid = t_eid;
        h_eid = w_eid;
      }

      int from, to;
      graph->eid2edge(h_eid, from, to);
      new_adjacency[from][to]     = true;
      new_adjacency[to][from]     = true;

      if (graph->eid_cost(k_eid) < graph->eid_cost(m_eid))
        m_eid = k_eid;
    }
  }
  t_eid = m_eid;
}

/************************************************************//**
 * @brief	
 * @version						v0.01b
 * Algorithm:
 *  http://www.sciencedirect.com/science/article/pii/0022000078900223
 ****************************************************************/
int MST::insert (int vid)
{
  #ifdef DEBUG
    printf("MST::insert(%d)\n", vid);
    printf("  |- MST.size() = %d\n", _size);
    printf("  |- subset.size() = %d\n", subset.size());
    for (int j=0; j<subset.size(); j++)
      if (subset[j])
        printf("  |- subset[%d] = true\n", j);
  #endif
  if (_size == 0) {
    subset[vid] = true;
    _size++;
    _cost = 0;
  } else if (_size == 1) {
    subset[vid] = true;
    _size++;
    for (int vid00=0; vid00<graph->size(); vid00++) {
      for (int vid01=0; vid01<graph->size(); vid01++) {
        if (vid00 != vid01 && subset[vid00] && subset[vid01]) {
          adjacency[vid00][vid01] = true;
          adjacency[vid01][vid00] = true;
          _cost = graph->edge_weight[vid00][vid01];
          break;
        }
      }    
    }
  } else {
    z_vid           = vid;
    int t_eid       = -1;
    new_subset      = vector<bool>(graph->size(), false);
    new_subset[vid] = true;
    new_adjacency   = vector< vector<bool> >(graph->size(), (vector<bool>(graph->size(), false)));
    
    int vid00(0);
    for (; vid00<subset.size(); vid00++) {
      if (subset[vid00] && vid00 != vid)
        break;
    }

    this->_insert(vid00, t_eid);
    subset              = new_subset;
    adjacency           = new_adjacency;
    subset[vid]         = true;
    int from, to;
    graph->eid2edge(t_eid, from, to);
    adjacency[from][to] = true;
    adjacency[to][from] = true;

    _size = 0;
    _cost = 0;
    for (int vid00=0; vid00<graph->size(); vid00++) {
      if (subset[vid00])
        _size++;
      for (int vid01=vid00+1; vid01<graph->size(); vid01++) {
        if (adjacency[vid00][vid01]) {
          _cost += graph->edge_weight[vid00][vid01];
          #ifdef DEBUG
            printf("  |- mst edge <%d,%d>.cost() = %f\n", vid00, vid01, graph->edge_weight[vid00][vid01]);
          #endif
        }

      }
    }
  }

  return _cost;
}



