//
//  main.c
//  sandbox15
//
//  Created by Daniel Rehman on 2002134.
//
#include <stdio.h>
#include <string.h>
#include <math.h>
typedef unsigned long long nat;
nat ua_exists(nat m, nat n) {
    return powl(m, n) > 1;
}
void ua(nat m, nat n) {
    const nat k = 2 * n + 1;
    const nat s = powl(2 * m, k);
    const nat t = powl(2 * m, 2 * k);
    const nat S = powl(s, n);
    printf(" running UA with: \n\tm = %llu, \n\tn = %llu, \n\tk = %llu, \n\ts = %llu, \n\tS = %llu, \n\tt = %llu \n", m, n, k, s, S, t);
    nat g[S], h[S];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (nat i = 0; i < t; i++)  {
        for (nat j = 0; j < S; j++) {
            memcpy(h, g, sizeof h);
            for (nat k = 1; k < S; k *= s) {
                const nat hr = h[j + k * ((j / k + 1) % s - j / k % s)];
                const nat hl = h[j + k * ((j / k + s - 1) % s - j / k % s)];
                const nat hc = h[j];
                nat e = 1;
                for (nat l = 0; l < m; l++) e *= hr + hl + hc;
                g[j] = (g[j] + e) % m;
            }
        }
    }
}


void ua_new(nat m, nat n) {
    const nat k = 2 * n + 1;
    const nat s = powl(2 * m, k);
    const nat t = powl(2 * m, 2 * k);
    const nat S = powl(s, n);
    printf(" running UA with: \n\tm = %llu, \n\tn = %llu, \n\tk = %llu, \n\ts = %llu, \n\tS = %llu, \n\tt = %llu \n", m, n, k, s, S, t);
    nat g[S], h[S];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (nat o = 0; o < t; o++)  {
        for (nat i = 0; i < S; i++) {
            memcpy(h, g, sizeof h);
            for (nat j = 1; j < S; j *= s) {
                for (nat k = 1; k <= j; k *= s) {
                    for (nat l = 0; l < m; l++) {
                        g[i] *= h[i + k * ((i / k + s - 1) % s - i / k % s)] + powl(h[i], l);
                    }
                }
                g[i] = (g[i] + h[i + j * ((i / j + 1) % s - i / j % s)] * h[i]) % m;
            }//j in n
        }//s
    }//t
}

// abcdef   gh ijkl mn o   pqrstuvwxyz

// iteration variables:  oijkl
// space variables:   gh
// parameters:   mn
// others that might be used: pq

int main() {
    
    const char* positive = "RUF";
    const char* negative = "LDB";
    nat n = 2;
    
    for (nat i = 0; i < n; i++) {
        for (nat j = 0; j <= i; j++) {
            printf("(%llu :: [%c])  ", j, negative[j]);
        }
        printf("|| (%llu :: [%c]) +\n", i, positive[i]);
    }
    
    /// RCL + UCL + UCD
    
    /** output:
     
     (0, 0)
     (1, 0)  (1, 1)
     
     */
    
    // it seems that this is the right for loop system, for what we want...?
    
    
    return 0;
}
