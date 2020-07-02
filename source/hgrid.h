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

/**
 maps a set of search(unknown) values into the h grid,
 in the right positions.
 
 indicies are a vector of indicies
 (ie, rule numbers, with the dimensional neighborhood ordering)
 of the h grid cases, which have "unknown" for their output.
 
 h is the output. search is the input vector, which is the current try
 (ie, an unreduce of z in Z)
 for rules which are unknown.
 this function simply maps the vlaues in that
 search array to the correct hgrid positions,
 by going through each index in indicies,
 and putting it where it needs to go.
 
 super wordy explanation.
*/
void map(vector h, vector search, vector indicies, nat H);


void print_hgrid(vector h, struct parameters p);
void print_m2n2_hgrid(vector h, struct parameters p);
void print_m3n1_hgrid(vector h, struct parameters p);

void load_m2n2_hgrid(const char* filename, struct context* c);
void load_m3n1_hgrid(const char* filename, struct context* c);
void load_hgrid(const char* filename, struct context* c);


#endif /* hgrid_h */
