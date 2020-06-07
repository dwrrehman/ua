//
//  ca.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "ca.h"

#include "structures.h"
#include "vector.h"
#include "io.h"
#include "hgrid.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

void initialize(vector g, nat m, nat L,
                enum initial_state_type initial) {
    fill(0, g, L);
    if (initial == empty_state) return;
    else if (initial == dot_state) g[0] = 1;
    else if (initial == repeating_state) {
        for (nat i = 0; i < L; i++) g[i] = i % 2 == 0;
    } else if (initial == random_state) {
        for (nat i = 0; i < L; i++) g[i] = rand() % m;
    }
}

void fill_neighbors(vector read_array, nat cell,
                    vector neighbors, nat L, nat space) {
    neighbors[0] = read_array[cell];
    nat y = 1;
    for (nat f = 1; f < L; f *= space) {
        neighbors[y++] = read_array
        [cell + f * ((cell / f + 1) % space
                     - cell / f % space)]; // R
        neighbors[y++] = read_array
        [cell + f * ((cell / f + space - 1) % space
                     - cell / f % space)]; // L
    }
}

nat measure_lifetime_for_hgrid(vector h, struct parameters* p) {
    
    const nat
        S = p->L,
        m = p->m,
        nc = p->nc,
        space = p->space,
        time = p->time;
    
    nat count = 0;
    element ns[nc], f[S], g[S], states[time];
    initialize(f, m, S, p->initial_state);
    
    for (nat t = 0; t < time; t++) {
        memcpy(g, f, sizeof g);
        for (nat s = 0; s < S; s++) {
            fill_neighbors(g, s, ns, S, space);
            f[s] = h[unreduce(ns, m, nc)];
            const nat r = unreduce(g, m, S);
            if (!contains(states, count, r))
                states[count++] = r;
        }
    }
    return count;
}
