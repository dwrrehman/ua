//
//  main.cpp
//  search1
//
//  Created by Daniel Rehman on 1909043.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "structures.h"
#include "utilities.hpp"
#include "parameters.hpp"

#include "io.hpp"
#include "ca_util.hpp"
#include "symbolic.hpp"
#include "visualizer.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>


h_grid determine_h_grid(const expression& given, const nat m, const parameters& u) {    
    h_grid result(u.H, 0);
    for (nat h = 0; h < u.H; h++) {
        neighborhood ns(u.nc, 0);
        reduce(ns, h, m, u.nc);
        result[h] = evaluate(given, ns, m);
    }
    return result;
}


const h_grid rule_110 = { 0, 1, 1, 1, 0, 1, 1, 0 };

void process(expression e, const parameters& u) {
    
    const auto m_ary = determine_h_grid(e, u.m, u);
    const auto binary = determine_h_grid(e, 2, u);
    
    if (u.n == 1) {
        if (h_grids_equal(binary, rule_110)) {
            // yay!
            // we have a candidate.
        } else {
            // errror
        }
    } else {
        
    }
}

void try_terminals(expression e, expression head, const parameters& u) {
    for (nat m = 0; m < u.m; m++) {
        head = {expression_type::constant, m};
        process(e, u);
    }
    
    for (nat j = 0; j < u.nc; j++) {
        head = {expression_type::variable, 0, j};
        process(e, u);
    }
    
    ///TODO: this function is wrong.
}


void loop_over_all_expressions(expression e, expression& head, const parameters& u, nat depth) {
    if (depth > u.max_depth) return; 

    if (head.children.empty()) {
        head = {expression_type::operator_, 0, 0, operator_type::add, {{}, {}}, false};
    }
    
    for (auto& child : e.children) {
        loop_over_all_expressions(e, child, u, depth);
    }
}

inline static void search(const parameters u) {

    std::vector<score> scores = {};  
    
    std::cout << "searching the computational universe...\n";
    
    for (int bit = 1; bit < 31; bit++) { // DELETE ME: in favor of doing a symbolic search.

        h_grid h_grid;
        const nat z = unreduce(h_grid, u.m, u.H);
        const double score = simulate_lifetime(h_grid, u);
        
        if (score >= u.threshold) {             
            scores.push_back({score, z});
            std::cout << "FOUND (" << z << ") -----> " << score << " \n";
        }
    }
    write_scores(scores, u);
    print_results(scores, u);
}

int main(const int argc, const char * argv[]) {
    srand((unsigned) time(nullptr));
    auto parameters = compute_parameters(argv, argc);    
    if (parameters.mode == search_mode) search(parameters);
    else if (parameters.mode == visualize_mode) visualize(parameters);
    else print_usage();
}




/**
for (int node = 0; node < branching_node::count; node++) {
    
    if (node == branching_node::add_op) {
        
        
    } else if (node == branching_node::subtract_op) {
        
        
    } else if (node == branching_node::multiply_op) {
        
        
    } else if (node == branching_node::equals_op) {            
        
        
    } else abort();
}    
*/
