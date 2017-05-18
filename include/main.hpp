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
