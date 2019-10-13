//
//  ca_util.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "ca_util.hpp"

#include "structures.h"
#include "utilities.hpp"

#include <vector>


void initialize(std::vector<nat>& g, const parameters& u) {
    g = std::vector<nat> (u.L, 0);
    if (u.initial_state == initially::empty) return;    
    else if (u.initial_state == initially::dot) g[0] = 1;    
    else if (u.initial_state == initially::repeating) {
        for (nat i = 0; i < u.L / 2; i++) {
            g[i] = 0;
            g[i + 1] = 1;
        }
    } else if (u.initial_state == initially::random) {
        for (nat i = 0; i < u.L; i++) g[i] = rand() % u.m;
    } 
}

void fill_neighbors(vec h, nat j, std::vector<nat>& ns, const parameters& u) {
    ns[0] = h[j];
    nat y = 1;
    for (int f = 1; f < u.L; f *= u.size) {
        ns[y++] = h[j + f * ((j / f + 1) % u.size - j / f % u.size)];
        ns[y++] = h[j + f * ((j / f + u.size - 1) % u.size - j / f % u.size)];
    }
}

nat simulate_lifetime(vec H_grid, const parameters& u) {
    
    std::vector<nat> g(u.L, 0), h(u.L, 0), ns(u.nc, 0), states {};
    
    initialize(g, u);
    
    while (true) {
        h = g;
        for (nat j = u.L; j--;) {
            fill_neighbors(h, j, ns, u);            
            g[j] = H_grid[unreduce(ns, u.m, u.nc)];
        }
        const nat r = unreduce(h, u.m, u.L);
        if (not_in(states, r)) states.push_back(r);        
        else break;
    }
    return states.size();
}
