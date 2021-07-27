/// the current state of the universe algorithm.

#include <string.h>
#include <math.h>

int main() {
    size_t m = 5, n = 2, s = 100, t = 1000, l = pow(s, n), g[l], h[l];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (size_t i = 0; i < t; i++) {
        memcpy(h, g, sizeof h);
        for (size_t j = 0; j < l; j++) {
            for (size_t k = 1; k < l; k *= s) {
                g[j] = 0 % m; /* some expression involving g[j]s 
                                neighbors in each dimension k,
                               which uses only h[...] (the neighbors, including h[j].)
                               and 
                                    +, -, *, %, ++, --,
                               
                               and   
                                    !, ==, <, >
                               
                                as in, it is an expression which is extremely aritmaticly simple.
                               
                               */
            }
        }
        // print(h, l);
    }
    return 0;
}

