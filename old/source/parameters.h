//
//  parameters.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//                                                       
//

#ifndef parameters_h
#define parameters_h

#include "structures.h"

bool is_mn_case(nat m, nat n, struct parameters p);

void print_parameters(struct parameters p);

void verbose_print_parameters(struct parameters p);

void set_parameter(struct parameters* p, const char* name, const char* value);

void load_parameters_from_file(const char* filename, struct context* p);

#endif /* parameters_h */
