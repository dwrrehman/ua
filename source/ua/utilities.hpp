//
//  utilities.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef utilities_hpp
#define utilities_hpp

#include "structures.h"


bool h_grids_equal(h_grid a, h_grid b);

nat unreduce(vec v, nat radix, nat length);
void reduce(std::vector<nat>& out, nat s, nat radix, nat length);

bool not_in(vec v, nat e);

#endif /* utilities_hpp */
