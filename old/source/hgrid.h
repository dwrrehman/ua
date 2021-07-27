//
//  hgrid.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef hgrid_h
#define hgrid_h

#include "structures.h"
#include "vector.h"
#include <stdbool.h>

nat to(nat base, nat exponent);
void map(vector hgrid, vector search, vector indicies, nat H);

void load_n5_hgrid(const char* filename, struct context* context);
void load_n4_hgrid(const char* filename, struct context* context);
void load_n3_hgrid(const char* filename, struct context* context);
void load_hgrid(const char* filename, struct context* context);

void print_hgrid(vector hgrid, struct parameters parameters, bool generic);

void filter_using_blacklist(char** input);

#endif /* hgrid_h */
