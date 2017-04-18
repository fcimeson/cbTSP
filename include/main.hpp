
/****************************************************************************//**
 ********************************************************************************
 * @file        subisosat.hpp
 * @brief       
 * @author      Frank Imeson
 * @date        
    ToDos:
      - implement LKH search for every solution (and first solution)
      - limit number of added clauses
      - limit number of added litterals
      - time out?
      - fix unsat bug
      - fix batch loop
 ********************************************************************************
 ********************************************************************************/

#ifndef MAIN_H		// guard
#define MAIN_H

/********************************************************************************
 * INCLUDE
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <boost/program_options.hpp>

#include "formula.hpp"
#include "tsp.hpp"
#include "sattsp.hpp"
#include "minisat/core/Solver.h"
#include "minisat/core/Dimacs.h"


using namespace std;
using namespace Minisat;
namespace po = boost::program_options;

/********************************************************************************
 * Defs
 ********************************************************************************/


/********************************************************************************
 * Prototypes
 ********************************************************************************/





#endif
