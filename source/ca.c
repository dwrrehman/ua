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

#include <stdlib.h>

void initialize(vector g, nat m, nat L, enum initial_state_type initial) {
    if (initial == empty_state) return;
    else if (initial == dot_state) g[0] = 1;
    else if (initial == repeating_state) {
        for (nat i = 0; i < L; i++) {
            g[i] = i % 2 == 0;
        }
    } else if (initial == random_state) {
        for (nat i = 0; i < L; i++) g[i] = rand() % m;
    }
}

void fill_neighbors(vector read_array, nat cell, vector neighbors, nat L, nat space) {
    neighbors[0] = read_array[cell];
    nat y = 1;
    for (nat f = 1; f < L; f *= space) {
        neighbors[y++] = read_array[cell + f * ((cell / f + 1) % space - cell / f % space)];
        neighbors[y++] = read_array[cell + f * ((cell / f + space - 1) % space - cell / f % space)];
    }
}
