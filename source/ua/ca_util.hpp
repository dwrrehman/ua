//
//  ca_util.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef ca_util_hpp
#define ca_util_hpp

#include "utilities.hpp"

#include <vector>

void initialize(std::vector<nat>& g, const parameters& u);

void fill_neighbors(std::vector<nat> h, nat j, std::vector<nat>& ns, const parameters& u);

nat simulate_lifetime(std::vector<nat> H_grid, const parameters& u);

#endif /* ca_util_hpp */
