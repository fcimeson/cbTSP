# cbTSP
A high level path planning solver for the SAT-TSP problem described in "An SMT-Based Approach to Motion Planning for Multiple Robots with Complex Constraints", T-RO, under review.


# Citing this work
Currently the paper for this work is under review.


# Installation/Dependencies
The software is written in C++ and depends on the Boost program_options library as well as the Boost regex library. cbTSP utilizes the SAT solver minisat and the TSP solver LKH, both of which are packaged into this project.

To compile cbTSP, as well its dependencies (minisat and LKH), type make. 


# Using the solver
To run cbTSP, specify the filename/location of the sat and tsp instance. Example:
```
 $ cbTSP dir/sat_instance.cnf dir/tsp_instance.tsp
```
Alternatively
```
 $ cbTSP dir/sat_instance
```
will load dir/sat_instance.cnf and dir/tsp_instance.tsp as the input instance. The configuration of cbTSP is done through the commnad line, which is documented below and in the paper.
```
$ cbTSP --help
Usage input_files [options]:
  --input arg                   input instance file(s)
  -o [ --output ] arg           output filename
  --brute                       enable brute mode (cb_interval > |V|)
  --max_time arg                max trial time (seconds)
  --max_usat_time arg           max trial for each sat instance (seconds)
  --max_tsp_cost arg (=-1)      max tsp cost
  --max_subgraph_cost arg (=-1) max subgraph cost
  --bdiv arg (=10)              set binary search divider
  --cb_interval arg (=1)        tsp callback interval
  --conflicts arg (=-1)         (-1 for unlimited)
  --propagations arg (=-1)      (-1 for unlimited)
  --linear_search               enable linear search
  --tsp_monotonic               assume graph is tsp monotonic
  --non_tspMonotonic            assume graph is not tsp monotonic (brute mode)
  --input_lkh_params arg        input LKH parameter file
  --print_lkh_params            print default lkh parameters
  -v [ --verbose ] arg (=1)     verbose level
  -h [ --help ]                 produce help message
```


# Known Issues
Currently the solver makes external calls to the LKH. This is a bottle neck and I have plans to compile LKH into cbTSP to address this.


# Licence
Copyright 2017 Frank Imeson and Stephen L. Smith
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


# Contact information
Frank Imeson  
Department of Electrical and Computer Engineering  
University of Waterloo  
Waterloo, ON Canada  
web: https://ece.uwaterloo.ca/~fcimeson/  
email: fcimeson@uwaterloo.ca  
