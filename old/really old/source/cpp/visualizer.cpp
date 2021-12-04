//
//  visualizer.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//

#include "visualizer.hpp"

#include "utilities.hpp"
#include "parameters.hpp"

#include "io.hpp"
#include "ca_util.hpp"
#include "symbolic.hpp"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <thread>

bool quit = false;
bool stop = false;
bool pause_ = false;

void input() {
    while (not quit) {
        char c = getch();
        if (c == 'q') quit = true;        
        if (c == 'c') stop = true;        
        else if (c == 'p') pause_ = true; 
        else if (c == 'u') pause_ = false;
    }
}

void visualize(const parameters& u) {
        
    std::vector<nat> g(u.L, 0), h(u.L, 0);
    std::vector<nat> ns(u.nc, 0);
    auto z_values = read_nats(u.source);    
    std::thread input_thread(input);
    
    for (auto z : z_values) {
        
        if (quit) {quit = false; break; }
        std::cout << "----- vizualizing z = " << z << ": -------- \n";
        
        std::vector<nat> h_grid(u.H, 0);
        reduce(h_grid, z, u.m, u.H);
        print(h_grid, "reduced: ");        
        initialize(g, u);
        
        for (nat t = 0; t < u.lifetime; t++) {
            if (stop) { stop = false; break; }
            while (pause_) sleep(1);
            
            h = g;
            for (nat i = 0; i < u.L; i++) {
                fill_neighbors(h, i, ns, u);
                g[i] = h_grid[unreduce(ns, u.m, u.nc)];
            }            
            print(h, t, u);
            usleep((unsigned) u.delay);
        }
        sleep(1);
    }    
    input_thread.join();
}
