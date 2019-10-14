//
//  structures.h
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef structures_h
#define structures_h

#include <string>
#include <vector>
#include <stdlib.h>

using nat = size_t;

enum mode {
    search_mode,
    visualize_mode,
};

namespace initially {
    enum initial_configuration {
        empty,
        dot,
        repeating,
        random,
    };
}

struct score {
    double score;
    nat z;
};


struct parameters {
    
    /// the modulus.
    nat m = 0;
    
    /// the dimension.
    nat n = 0; 
    
    /// the side length of the ncube.
    nat size = 0;
    
    /// the lifetime duration for visualization.
    nat lifetime = 0;
    
    /// the delay in microseconds for visualization.
    nat delay = 0;
    
    /// set to 1 for visualization, 0 for search.
    nat mode = 0;
    
    /// the maximum expression depth for symbolic search.
    nat max_depth = 0;    
        
    /// the search score threshold to save to a file.
    double threshold = 0;
    
    /// should visualize the CA n dimensionally, as opposed to serialize over time.
    bool n_dimensional_display = false;
    
    /// print the CA as a set of numbers.    
    bool numeric_print = false;
    
    /// the initial configuration of the space.
    nat initial_state = initially::empty;
    
    /// the file path destintation for the scores and z values found using search method.
    std::string destination;
        
    /// the file path source used to vizualize sequential automatons.
    std::string source;
    
    // -------------  derived: ---------------
    
    /// nc = 2n + 1 = "neighborhood count"
    nat nc = 0;
    
    /// L = size ^ n = "total cells"
    nat L = 0;
    
    /// H = m ^ nc = "neighborhood state count"
    nat H = 0;
    
    /// big H = m ^ L = "total CA state count" 
    nat big_H = 0;
    
    /// Z = m ^ H = "total possible number of definitions"
    nat Z = 0;    
};

enum class expression_type {
    null,
    operator_, 
    constant, 
    variable,
    paren,
};

enum class operator_type { 
    null,
    add, 
    subtract, 
    multiply, 
    equals,
};

namespace branching_node {
    enum type {
        add_op,
        subtract_op,
        multiply_op,
        equals_op,
        count,
    };
}

/// an expression literal takes the form;       
/// : 
///      expression {type, value, index, operator, children, error};

struct expression {
    enum expression_type type = expression_type::null;
    nat constant_value = 0;  // valid if type == constant.
    nat variable_index = 0;  // valid if type == variable
    enum operator_type operator_ = operator_type::null; // valid if type == operator.
    std::vector<expression> children = {}; // nonempty if type == operator.  has length 2.
    bool error = false;
};

struct token {
    expression_type type = expression_type::null;
    operator_type operator_ = operator_type::null;
    nat value = 0;
    bool open = false;
};

#endif /* structures_h */
