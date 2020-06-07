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

void initialize(vector g, nat m, nat L, enum initial_state_type initial);

void fill_neighbors(vector h, nat j, vector ns, nat L, nat space);

nat measure_lifetime_for_hgrid(vector h, struct parameters* p);

#endif /* ca_h */
