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
    random_state,
    center_dot_state,
};

enum display_type {
    no_display,
    numeric_display,
    intuitive_display,
    binary_display,
};

struct parameters {
    
    /// the modulus.
    nat m;
    
    /// the neighborhood count.
    nat n;
    
    /// the number of cells.
    nat s;

    /// the number of timesteps. (ie, lifetime duration, for visualization.
    nat t;
    
    /// the sidelength of the (d dimensional) cube of s cells.
    nat l;
    
    /// the delay in microseconds for visualization.
    nat delay;
            
    /// the initial configuration of the space.
    enum initial_state_type initial_state;
        
    /// display type:   none, numeric, gradient,
    enum display_type display_as;
        
    /// should visualize the CA n dimensionally, as opposed to serialize over time.
    bool n_dimensional_display;
    
    
//    // ------ computed parameters: ------
//    /// m_to_n = H = m ^ n = "number of the possible neighborhood arrangments".
//    nat H;   /// DELETED.
};

struct rule {
    /// Always has count of nc.
    /// and, is always ordered according to the
    /// "Natrual Ordering", which is:
    ///
    ///          C  L R  U D  F B  A P     etc...
    ///
    nat* neighborhood;
    
    /// the future timestep new cell value, given that neighborhood.
    nat future;
};

struct context {
    const char* home;
    struct parameters parameters;    
    vector hgrid;
    nat z;
};

#endif /* structures_h */
