/// current state of the ua theory.

#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll()

#include <string.h> // using memset(), memcpy().
#include <math.h>   // using powl()
typedef unsigned long long nat;



int main(int argc, const char** argv) {
    if (argc <= 6) { printf("4: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        s = atoll(argv[3]), S = powl(s, n),
        t = atoll(argv[4]),
        d = atoll(argv[5]),
        nd = atoll(argv[6]);
    
    nat g[S], h[S], x[2 * n + 1];
    memset(g, 0, sizeof g);
    
    if (n == 2)
        g[S / 2 + s / 2] = 1;
    
    else g[0] = 1;
    
    for (nat i = 0; i < t; i++) {
        if ((i % 200 == 0 && i) || nd) printf("\e[1;1H\e[2J");
        memcpy(h, g, sizeof h);
        for (nat j = 0; j < S; j++) {
            
            x[0] = h[j];
            nat y = 1;
            for (nat f = 1; f < S; f *= s) {
                x[y++] = h[j + f * ((j / f + 1) % s - j / f % s)];
                x[y++] = h[j + f * ((j / f + s - 1) % s - j / f % s)];
            }
                                                            
            const nat c = x[0];
            const nat r = x[1];
            const nat l = x[2];
            const nat u = x[3];
            const nat d = x[4];
            
            nat t21 = 1;
            t21 = t21 * l + 1;
            t21 = t21 * c + 1;
            t21 = t21 * r + 0;
                    
            nat t22 = 1;
            t22 = t22 * d + 1;
            t22 = t22 * l + 1;
            t22 = t22 * c + 1;
            t22 = t22 * u + 0;
            
            g[j] = (g[j] + t21 + t22) % m;
            
            if (j % s == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[j] / m * 24) + 232);
            fflush(stdout);
        }
        printf("\n");
        usleep((unsigned) d);
    }
    return 0;
}
