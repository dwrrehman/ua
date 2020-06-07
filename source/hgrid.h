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
 maps a set of search(unknown) and
 known hgrid values into the h grid,
 in the right positions.

 ie, the search ones are interpolated around the known ones,
 according to the known indicies
 (ie, the positions of where the known should be.)

     note: assumes:     |search| + |known| = H.
 
*/
void map(vector hgrid, vector known, vector known_indicies, vector search, nat H);

void print_hgrid(vector h, struct parameters p);
void print_m2n2_hgrid(vector h, struct parameters p);
void print_m3n1_hgrid(vector h, struct parameters p);

void load_m2n2_hgrid(const char* filename, struct context* c);
void load_m3n1_hgrid(const char* filename, struct context* c);
void load_hgrid(const char* filename, struct context* c);



#endif /* hgrid_h */
