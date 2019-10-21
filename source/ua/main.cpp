//
//  main.cpp
//  search1
//
//  Created by Daniel Rehman on 1909043.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

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

#include <set>


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


void process(expression e, const parameters& u, std::vector<score> scores) {
    
    const auto m_ary = determine_h_grid(e, u.m, u);
    const auto binary = determine_h_grid(e, 2, u);

    if (u.n == 1) {
        if (h_grids_equal(binary, rule_110)) {
            abort();
        } else {
            
        }
    } else {
        
    }
}





std::set<std::string> found = {};


void print(expression e, const parameters& u, std::vector<score> scores) {
    
    auto wef = found.insert(stringify(e));
    if (wef.second) {
        std::cout << pretty_stringify(e) << "\n";
        process(e, u, scores);
    }

}


void generate(expression& e, expression& head, const parameters& u, nat depth, std::vector<score> scores) {
    
    print(e, u, scores);
    
    if (depth > u.max_depth) return;
    

    for (nat l = 0; l < u.m + u.nc; l++) {
        for (nat r = 0; r < u.m + u.nc; r++) {
            for (nat o = 0; o < branching_node::count; o++) {
        
                nat L = l;
                nat R = r;
    
                expression_type l_type = expression_type::constant;
                expression_type r_type = expression_type::constant;
                
                if (r >= u.m) {
                    r_type = expression_type::variable;
                    R = r - u.m;
                }
                
                if (l >= u.m) {
                    l_type = expression_type::variable;
                    L = l - u.m;
                }
                                
                expression add = {};
                
                if (o == branching_node::add_op) {
                    add = expression {
                        .type = expression_type::operator_,
                        .operator_ = operator_type::add,
                        .children = {
                            {l_type, L, L},
                            {r_type, R, R}
                        }
                    };
                    
                } else if (o == branching_node::subtract_op) {
                    add = expression {
                        .type = expression_type::operator_,
                        .operator_ = operator_type::subtract,
                        .children = {
                            {l_type, L, L},
                            {r_type, R, R}
                        }
                    };
                } else if (o == branching_node::multiply_op) {
                   add = expression {
                       .type = expression_type::operator_,
                       .operator_ = operator_type::multiply,
                       .children = {
                           {l_type, L, L},
                           {r_type, R, R}
                       }
                   };
                } else if (o == branching_node::equals_op) {
                   add = expression {
                       .type = expression_type::operator_,
                       .operator_ = operator_type::equals,
                       .children = {
                           {l_type, L, L},
                           {r_type, R, R}
                       }
                   };
                }
                
                
                head = add; // left
                generate(e, head.children[0], u, depth + 1, scores);
                
                head = add; // right
                generate(e, head.children[1], u, depth + 1, scores);
                
                head = add; // both
                generate(e, head.children[0], u, depth + 1, scores);
                generate(e, head.children[1], u, depth + 1, scores);
            }
        }
    }
    
    
    

    
   
}






































// ( C )
// ( V )


// ( e + e )
// ( e * e )
// ( e - e )
// ( e = e )














//void try_terminals(expression e, expression& head, const parameters& u, std::vector<score> scores) {
//    for (nat m = 0; m < u.m; m++) {
//        head = {expression_type::constant, m};
//        process(e, u, scores);
//    }
//
//    for (nat j = 0; j < u.nc; j++) {
//        head = {expression_type::variable, 0, j};
//        process(e, u, scores);
//    }
//
///
//}










    
//    //
//    head = add;
//    generate(e, head.children[0], u, depth + 1, scores);
//
//    head = add;
//    generate(e, head.children[1], u, depth + 1, scores);
//    //
//
    
//    auto save = head;
//    head = add;
//    for (auto& child : head.children) {
//        head = add;
//        generate(e, child, u, depth + 1, scores);
//    }
    













inline static void search(const parameters& u) {

    std::vector<score> scores = {};  
    
    std::cout << "searching the computational universe...\n";

    ///TODO: fix me: put me into the process func?
    h_grid h_grid;
    const nat z = unreduce(h_grid, u.m, u.H);
    const double score = simulate_lifetime(h_grid, u);
    
    if (score >= u.threshold) {             
        scores.push_back({score, z});
        std::cout << "FOUND (" << z << ") -----> " << score << " \n";
    }
    
    write_scores(scores, u);
    print_results(scores, u); 
}

int main(const int argc, const char * argv[]) {
    

    parameters u = {};
    expression e = {};
    
    u.m = 2;
    u.n = 1;
    u.nc = 3;
    u.max_depth = 2;
    generate(e, e, u, 0, {});
    
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
