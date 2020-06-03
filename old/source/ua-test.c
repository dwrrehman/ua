#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define something 0
typedef unsigned long long nat;
int main(int argc, const char** argv) {
    if (argc <= 6) { printf("usage: \n\t./ca m n s t d nd\n\n"); return 1; }
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        s = atoll(argv[3]), S = powl(s, n),
        t = atoll(argv[4]),
        d = atoll(argv[5]),
        nd = atoll(argv[6]);
    
    nat g[S], h[S];
    memset(g, 0, sizeof g);
    g[0] = 1;
    
    for (nat o = 0; o < t; o++) {
        
        if ((o % 200 == 0 && o) || nd) printf("\e[1;1H\e[2J");
        
        memcpy(h, g, sizeof h);
        for (nat i = 0; i < S; i++) {
            for (nat j = 1; j < S; j *= s) {
                nat e = h[i];
                for (nat k = 1; k <= j; k *= s) {
                    e *= h[i + k * ((i / k + s - 1) % s - i / k % s)] + 1;
                }
                e++;
                g[i] = (g[i] + h[i + j * ((i / j + 1) % s - i / j % s)] * e) % m;
            }
            
            if (i % s == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[i] / m * 24) + 232);
            fflush(stdout);
        }
        printf("\n");
        usleep((unsigned) d);
    }
    return 0;
}



















//printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[j] / m * 24) + 232);
//fflush(stdout);
//printf("\n");
// usleep((unsigned) d);
//        d = atoll(argv[5]);
