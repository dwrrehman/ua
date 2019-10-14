//
//  utilities.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef utilities_hpp
#define utilities_hpp

#include "structures.hpp"
#include <vector>

bool h_grids_equal(std::vector<nat> a, std::vector<nat> b);

nat unreduce(std::vector<nat> v, nat radix, nat length);
void reduce(std::vector<nat>& out, nat s, nat radix, nat length);

bool not_in(std::vector<nat> v, nat e);

#endif /* utilities_hpp */
