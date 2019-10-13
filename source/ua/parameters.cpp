//
//  parameters.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "parameters.hpp"

#include "structures.h"

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <vector>


void debug_parameters(parameters u) {
    std::cout << "(m = " << u.m << ", n = " << u.n << "):\n";
    
    std::cout << "\t size = " << u.size << "\n";
    std::cout << "\t mode = " << u.mode << "\n";
    std::cout << "\t threshold = " << u.threshold << "\n";
    std::cout << "\t lifetime = " << u.lifetime << "\n";
    std::cout << "\t delay = " << u.delay << "\n";
    
    std::cout << "computed: \n";
    
    std::cout << "\t s^n = L = " << u.L << "\n";
    std::cout << "\t 2n + 1 = nc = " << u.nc << "\n";
    std::cout << "\t m^nc = H = " << u.H << "\n";
    std::cout << "\t m^L = big H = " << u.big_H << "\n";
    std::cout << "\t m^H = Z = " << u.Z << "\n\n";
    
    std::cin.get();
}


void print_usage() {
    printf("usage: ./program m n s i mode \n\n"           
           "\t- m : M (nat0) : the modulus, also the size of the CA. \n"
           "\t- n : N (nat0) : the number of dimensions in the CA.\n"               
           "\t- s : N-Cube Side-length (nat0) : How many cells you want to simulate the CA with. L = s^n\n"
           "\t- i : Initial confifguration (0/1/2/3) : 0 for empty, 1 for dot, 2, for repeating, 3 for random. \n"
           "\t- mode : Mode (0/1) : determine how to use this program.\n\n"
           
           "if mode == 0: ./program m n s mode thr md dest\n"           
           "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n"           
           "\t- max_depth (nat0) : the maximum depth of symbolic expressions which will be considered. \n"
           "\t- Destination (file) : all found z values will be written here (truncs the file.) \n\n"
           
           "if mode == 1: ./program m n s mode l d nd np source\n"           
           "\t- l : Lifetime (nat0) : the number of timesteps to simulate CAs for.\n"           
           "\t- d : delay (nat0) : the delay for usleep. eg, 100000 \n" 
           "\t- nd : N-dimensional display (0/1) : vizualize the CA N-dimensionally.\n"          
           "\t- np : Numeric print (0/1) : print the modnat values as numbers instead of hues.\n"
           "\t- Source (file) : file containing all z values which will be vizualized. \n\n"
           
           "for viz input: \n"
           "\t- q to stop the viz of all automatons \n" 
           "\t- c to continue to the next z value \n"
           "\t- p/u to pause and unpause the simulation.\n"
           
           "\n");
    exit(1);
}


parameters compute_parameters(const char** argv, const int argc) {
    
    parameters u;
    if (argc <= 5) print_usage();                
    
    // general parameters:
    u.m = atoi(argv[1]);
    u.n = atoi(argv[2]);
    u.size = atoi(argv[3]);
    u.initial_state = atoi(argv[4]);
    u.mode = atoi(argv[5]);
    
    if ((u.mode == search_mode and argc <= 8) or 
        (u.mode == visualize_mode and argc <= 10)) print_usage();                
    
    if (u.mode == search_mode) {
        u.threshold = atof(argv[6]);
        u.max_depth = atoi(argv[7]);
        u.destination = argv[8];
        
    } else if (u.mode == visualize_mode) {          
        u.lifetime = atoi(argv[6]);
        u.delay = atoi(argv[7]);
        u.n_dimensional_display = atoi(argv[8]); 
        u.numeric_print = atoi(argv[9]);
        u.source = argv[10];
    }
    
    // compute seoncdary parameters:
    u.nc = 2 * u.n + 1;                 // asuming double odd neighborhood.
    u.L = pow(u.size, u.n);    
    u.H = pow(u.m, u.nc);
    u.big_H = pow(u.m, u.L);
    u.Z = pow(u.m, u.H);
    
    debug_parameters(u);
    return u;
}







