//
//  main.cpp
//  sandbox24
//
//  Created by Daniel Rehman on 1909146.
//

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <vector>

using nat = size_t;

void print(nat* h, nat l, nat m, nat n, bool n_dimensional_display, bool numeric_print) {
    if (n_dimensional_display) printf("\e[1;1H\e[2J");
    for (size_t i = 0; i < l; i++) {
        if (numeric_print) printf("%4lu ", h[i]);
        else printf("\033[38;5;%lum██\033[0m", 232 + (size_t) (24.0 * ((double) h[i] / (double) m)));
        if (n_dimensional_display) {
            for (size_t q = 1; q < n; q++) {
                size_t d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");    
}


int main(int argc, const char * argv[]) {
    
    nat m = 2, n = 1;
    nat t = 10000;
    nat s = 100;
    unsigned d = 100000;
        
    nat g[s], h[s];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (nat i = 0; i < t; i++) {
        memcpy(h, g, sizeof h);
        for (int j = 0; j < s; j++) {
            nat l = h[(j + 1) % s];
            nat r = h[(j + s - 1) % s];
            nat c = h[j];
            g[j] = (r * c * l + r * c + r + c) % m;
        }
        print(h, s, m, n, 0, 0);
        usleep(d);
    }
    return 0;
}
