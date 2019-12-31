/// current state of the ua theory.

// extraneous:
#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll()

// actually needed:
#include <string.h> // using memset(), memcpy().
#include <math.h> // using powl()

int main(int argc, const char** argv) {
    if (argc <= 5) { printf("usage: \n\t./ca m n s t d\n\n"); return 1; }
    const unsigned long long
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        s = atoll(argv[3]), S = powl(s, n),
        t = atoll(argv[4]),
        d = atoll(argv[5]);
    
    unsigned long long g[S], h[S];
    memset(g, 0, sizeof g);
    g[0] = 1;
    
    for (unsigned long long i = 0; i < t; i++) {
        if (i % 200 == 0 && i) printf("\e[1;1H\e[2J");
        memcpy(h, g, sizeof h);
        for (unsigned long long j = 0; j < S; j++) {
                                    
            const unsigned long long l = h[(j + s + 1) % s];
            const unsigned long long r = h[(j + s - 1) % s];
            const unsigned long long c = h[j];
            
            const unsigned long long e = (     c * (l + 1)         ) % m;
            const unsigned long long u = (     m - e + m - 1       ) % m;
            
            g[j] -= r * u;
            g[j] %= m;
            
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[j] / m * 24) + 232);
        }
        printf("\n");
        usleep((unsigned) d);
    }
    return 0;
}












/// STORAGE FOR DEAD CODE:


//for (int f = 1; f < L; f *= l) {
//    g[j] += h[j + f * ((j / f + l + 1) % l - j / f % l)];
//    g[j] %= m;
//}


//
//const unsigned long long e = c * (l * l + l + 1);
//
//const unsigned long long u = r * (e * e + e + 1);
