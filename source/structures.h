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
    no_display,
    numeric_display,
    intuitive_display,
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
    
            
    // ------ computed parameters: ------
    
    /// nc = 2n + 1 = "neighborhood count"
    nat nc;
    
    /// L = size ^ n = "total cells"
    nat L;
    
    /// H = m ^ nc = "neighborhood state count"
    nat H;    
};


struct rule {
    /// Always has count of nc.
    /// and, is always ordered according to the
    /// "Natrual Ordering", which is:
    ///
    ///          C  R L  U D  F B  A P     etc...
    ///
    nat* neighborhood;
    
    /// the future timestep new cell value, given that neighborhood.
    nat future;
};


/// an h grid, aka, ruleset, that was specified by a .hg.txt file.
/// it might not contain a transition for every rule, and thus cannot
/// simply be a truer representation, which is just a vector of
/// modnats with length H.
struct ruleset {
    struct rule* rules;
    nat count;
};


struct context {
    const char* home;
    struct parameters parameters;    
    vector hgrid;
    nat z;
};



#endif /* structures_h */
