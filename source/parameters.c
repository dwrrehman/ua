//
//  parameters.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "parameters.h"

#include "structures.h"


#include <stdio.h>
#include <math.h>


void compute_derived_parameters(struct parameters* u) {
    u->nc = 2 * u->n + 1;
    u->H = powl(u->m, u->nc);
    u->L = powl(u->space, u->n);    
}


void print_parameters(struct parameters* u) {
    printf("current parameters: \n");
    printf("\tm = %llu\n", u->m);
    printf("\tn = %llu\n", u->n);
    printf("\tspace = %llu\n", u->space);
    printf("\ttime = %llu\n", u->time);
    printf("\tdelay = %llu\n", u->delay);
    printf("\tinitial_state_type = %d\n", u->initial_state);
    printf("\tn_dimensional_display = %d\n", u->n_dimensional_display);
    printf("\tdisplay_type = %d\n", u->display_as);
    printf("\tthr = %f\n", u->threshold);    
        
    printf("computed parameters:\n");
    printf("\tnc = %llu\n", u->nc);
    printf("\tH = %llu\n", u->H);
    printf("\tL = %llu\n", u->L);
    printf("\n");
}
