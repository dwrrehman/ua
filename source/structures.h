//
//  structures.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef structures_h
#define structures_h


#include <stdbool.h>


typedef unsigned long long int nat;
typedef signed long long int integer;
typedef nat element;
typedef element* vector;

enum initial_state_type {
    empty_state,
    dot_state,
    repeating_state,
    random_state
};

enum display_type {
    none,
    numeric,
    gradient
};





struct parameters {
    
    /// the modulus.
    nat m;
    
    /// the dimension.
    nat n;
    
    /// the side length of the n-cube. cell count will be space ^ n.
    nat space;
    
    /// the time, or lifetime duration, for visualization.
    nat time;
        
    /// the delay in microseconds for visualization.
    nat delay;
            
    /// the initial configuration of the space.
    enum initial_state_type initial_state;
        
    /// display type:   none, numeric, gradient,
    enum display_type display_as;
        
    /// should visualize the CA n dimensionally, as opposed to serialize over time.
    bool n_dimensional_display;
    
    /// a threshold used when searching.
    double threshold;
    
    
    
    // ------ computed parameters: ------
    
    /// nc = 2n + 1 = "neighborhood count"
    nat nc;
    
    /// L = size ^ n = "total cells"
    nat L;
    
    /// H = m ^ nc = "neighborhood state count"
    nat H;    
};


#endif /* structures_h */
