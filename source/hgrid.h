//
//  hgrid.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef hgrid_h
#define hgrid_h

#include "structures.h"
#include "vector.h"

// assuming :     |search| + |known| = H.
void map(vector h_grid,
         vector known,
         vector known_indicies,
         vector search, const struct parameters* u);



#endif /* hgrid_h */
