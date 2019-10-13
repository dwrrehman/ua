//
//  parameters.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef parameters_hpp
#define parameters_hpp

#include "structures.h"

void debug_parameters(parameters u);
void print_usage();
parameters compute_parameters(const char** argv, const int argc);

#endif /* parameters_hpp */
