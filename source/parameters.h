//
//  parameters.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef parameters_h
#define parameters_h

#include "structures.h"

bool is_mn_case(nat m, nat n, struct parameters p);

void compute_derived_parameters(struct parameters* p);

void print_parameters(struct parameters p);

void load_parameters(const char* filename, struct context* p);

#endif /* parameters_h */
