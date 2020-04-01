/// current state of the ua theory.

#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll()

#include <string.h> // using memset(), memcpy().
#include <math.h>   // using powl()
typedef unsigned long long nat;

int main(int argc, const char** argv) {
    if (argc <= 6) { printf("11: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        s = atoll(argv[3]), S = powl(s, n),
        t = atoll(argv[4]),
        d = atoll(argv[5]),
        nd = atoll(argv[6]);
    
    nat g[S], h[S], pos[n], neg[n];
    
    memset(g, 0, sizeof g);
    if (n == 2) g[S / 2 + s / 2] = 1;
    else ++*g;
    
    for (nat _i = 0; _i < t; _i++) {
        if ((_i % 200 == 0 && _i) || nd) printf("\e[1;1H\e[2J");
        memcpy(h, g, sizeof h);
        for (nat _j = 0; _j < S; _j++) {
            
            nat y = 0;
            for (nat f = 1; f < S; f *= s) {
                pos[y] = h[_j + f * ((_j / f + 1) % s - _j / f % s)];
                neg[y] = h[_j + f * ((_j / f + s - 1) % s - _j / f % s)];
                y++;
            }
    
            const nat c = h[_j];
            
            const nat r = pos[0];
            const nat l = neg[0];
            
            
            
            const nat d = neg[1];
            const nat u = pos[1];
            
    
            const nat _22 = (
                             
                                u  *  ( c * (l + 1) * (d + 1) + 1 )
                             
                             ) % m;
            const nat r110 = (r * (c * (l + 1) + 1)) % m;
            
            g[_j] = (g[_j] + r110 + _22) % m;
                                    
            if (_j % s == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[_j] / m * 24) + 232);
            fflush(stdout);
        }
        printf("\n");
        usleep((unsigned) d);
//        getc(stdin);
    }
}

//            g[_j] = (h[_j] + pos[0] * (h[_j] * (neg[0] + 1) + 1 )) % m;   /// rule 110.     (2,1) case.
