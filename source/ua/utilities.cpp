//
//  utilities.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "utilities.hpp"

#include "structures.h"

#include <vector>
#include <algorithm>


bool h_grids_equal(h_grid a, h_grid b) {
    if (a.size() != b.size()) return false;    
    for (nat i = 0; i < a.size(); i++) 
        if (a[i] != b[i]) return false;
    return true;
}

nat unreduce(vec v, nat radix, nat length) {
    nat s = 0, p = 1;
    for (nat i = 0; i < length; i++, p *= radix) 
        s += p * v[i];
    return s;
}

void reduce(std::vector<nat>& out, nat s, nat radix, nat length) {    
    for (nat i = 0, p = 1; i < length; i++, p *= radix) 
        out[i] = (s / p) % radix;
}

bool not_in(vec v, nat e) {
    return std::find_if(v.begin(), v.end(), [=](auto i) {return i == e;}) == v.end();
}
