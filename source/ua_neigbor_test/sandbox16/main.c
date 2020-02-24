//
//  main.c
//  sandbox16
//
//  Created by Daniel Rehman on 2002171.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

/// tests the positive and negative neighbor-computation for the ua.

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef unsigned long long nat;

/// abcdef gh ijkl mn o pq r st uvwxyz
///        ** **** **

void print_space(const nat* b, const nat k, const nat s, const nat S, const nat n) {
    for (nat i = 0; i < S; i++) {
        
        bool is_neighbor = false;
        
        if (i % s == 0 && s && n == 2) printf("\n");
        if (i % s == 0 && s && n == 3) printf("   ");
        if (i % (s * s) == 0 && s && n == 3) printf("\n");
        
        for (nat j = 0; j < k; j++) {
            if (i == b[j]) {
                is_neighbor = true;
            }
        }
        
        if (!is_neighbor) printf("[ ]");
        else printf("[x]");
    }
    printf("\n");
}



static void print_vector(const nat* v, const nat n) {
    printf("[ ");
    for (nat i = 0; i < n; i++) {
        printf("%llu ", v[i]);
    }
    printf("]\n");
}





int main(int argc, const char * argv[]) {
    
    
    const nat
        n = 3,
        s = 5, S = powl(s, n);
    
    const nat k = 2 * n + 1;
    
    nat x[k];
    memset(x, 0, sizeof x);
        
    printf("s = %llu, S = %llu, k = %llu, n = %llu\n", s, S, k, n);
    
    for (nat i = 0; i < S; i++) {

//        nat j = 13;
    
        x[0] = i;
        nat y = 1;
        for (nat j = 1; j < S; j *= s) {
            
            x[y++] = i+j*((i/j+1)%s-i/j%s);
            x[y++] = i+j*((i/j+s-1)%s-i/j%s);
            
        }
        
        print_vector(x, k);
        
        print_space(x, k, s, S, n);
        
        printf("\n");
    }
    
    return 0;
}
