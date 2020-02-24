//
//  main.c
//  ua-draft0
//
//  Created by Daniel Rehman on 2002226.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//
#include <string.h> ///TODO: get rid of these.
#include <math.h>
#include <stdio.h>

typedef unsigned long long nat;

int main() {
    
    const nat m = 2, n = 2, s = powl(m * m, 2 * n + 1), t = powl(m * m * m, 3 * n + 1);
    
    
    printf("%llu, %llu -> s = %llu, t = %llu\n", m,n,s,t);
    //return 1;
    
    
    
    nat g[s], h[s];
    memset(g, 0, sizeof g);
    ++*g;
    
    for (nat f = 0; f < t; f++) {
        
        for (nat i = 0; i < s; i++) {
            
            memcpy(h, g, sizeof h);
            
            for (nat j = 0; j < n; j++) {
                
                for (nat k = 0; k <= j; k++) {
                    
                    for (nat l = 1; l < m; l++) {
                        // something here;
                    }
                }
                
                for (nat k = 1; k < m; k++) {
                    // something here
                }
                
                g[i] += 0; // pos[i] * (something here)
            }
            
            
            printf("%llu ", h[i]);
        }
        printf("\n");
    }

    return 0;
}
