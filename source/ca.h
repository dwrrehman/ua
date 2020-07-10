//
//  ca.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef ca_h
#define ca_h

#include "structures.h"

enum mode_type {stopped, running, paused, stepping};

extern const nat unknown_dummy_value;
extern char mode;

void handler(int e);

void graph(nat x, nat m);

void initialize(vector g, nat m, nat n, nat L, nat s,
enum initial_state_type initial);

void fill_neighbors(vector h, nat j, vector ns, nat L, nat space);

nat measure_lifetime(vector h, struct parameters* p);

void threshold_search(nat threshold, const char* outfile, struct context* c);

void visualize_lifetime(nat begin, nat begin_slice, nat end_slice, vector h, struct parameters p);

extern void user_interface(struct context *context);

void visualize_set(nat begin,
                   
                   nat user_begin_slice,
                   nat user_end_slice,
                   
                   vector set, nat count,
                   
                   const char* savelist_out_filename,
                   const char* blacklist_out_filename,
                   
                   struct context* context);


void generate_lifetime_image(const char* filename,
                             nat begin,
                             
                             nat begin_slice,
                             nat end_slice,
                             
                             vector h,
                             struct parameters p);



#endif /* ca_h */
