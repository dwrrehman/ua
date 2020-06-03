//
//  hgrid.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "hgrid.h"

#include "structures.h"
#include "vector.h"

#include <stdio.h>
#include <assert.h>


void print_h_grid(vector h_grid, const struct parameters* u) {
    element ns[u->nc];
    printf("printing generic h_grid: \n");
    for (nat h = 0; h < u->H; h++) {
        reduce(ns, h, u->m, u->nc);
        print_vector(ns, u->nc);
        printf(" ---> %llu\n", h_grid[h]);
    }
    printf("\n");
}


void print_m2n2_h_grid(vector h_grid, const struct parameters* u) {
    assert(u->m == 2 && u->n == 2 && "trying to print (2,2) h grid, wrong m and n...");
//    element ns[5];
    
    printf("unimplemented\n");
}

void print_m3n1_h_grid(vector h_grid, const struct parameters* u) {
    assert(u->m == 3 && u->n == 1 && "trying to print (3,1) h grid, wrong m and n...");
//    element ns[3];
    printf("unimplemented\n");
}


void map(vector h_grid, vector known, vector known_indicies, vector search, const struct parameters* u) {    
    nat j = 0, k = 0;
    for (nat i = 0; i < u->H; i++) h_grid[i] = i == known_indicies[j] ? known[j++] : search[k++];
}
