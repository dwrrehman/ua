//
//  header.h
//  universe
//
//  Created by Daniel Rehman on 1808083.
//

#ifndef header_h
#define header_h


enum fields {
    higgs,
    gluon,
    photon,
    Z_boson,
    Wp_boson,
    Wn_boson,
    // ...
    field_count
};


/// Structures:

struct point {
    long signed int curvature;
    long int* fields;
};

#endif /* header_h */
