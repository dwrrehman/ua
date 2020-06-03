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


void initialize(vector g, const nat m, const nat L, enum initial_state_type initial) {
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

void fill_neighbors(vector h, nat j, vector ns, const nat L, const nat size) {
    ns[0] = h[j];
    nat y = 1;
    for (nat f = 1; f < L; f *= size) {
        ns[y++] = h[j + f * ((j / f + 1) % size - j / f % size)];
        ns[y++] = h[j + f * ((j / f + size - 1) % size - j / f % size)];
    }
}
