/// current state of the ua theory.

#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll()

#include <string.h> // using memset(), memcpy().
#include <math.h>   // using powl()
typedef unsigned long long nat;

int main(int argc, const char** argv) {
    if (argc <= 6) { printf("13: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
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
            
    
            const nat f =
            
//            l+
//            u+
//            l*c+
//            l*u+
//            c*u+
//            d*u+
//            l*c*u+
//            c*d*u+
//            l*c*d*u+
            
            //======================
            
//            l+
//            u+
//            l*r+
//            l*c+
//            l*u+
//            u*c+
//            l*c*r+
//            l*u*r+
//            l*c*u+
            
            
            // 1d term
            l+
            l*r+
            l*c+
            l*c*r+
            
            // 2d term
            u*c*d+
            u*l*d+
            u*l*r*d+
            u*l*c*d+
            u*l*c*r*d+
            u*c+
            u*l+
            u*l*r+
            u*l*c+
            u*l*c*r+
            u*d+
            u+
            
            
//            u+
////            u*d+
//            u*l*d+
//            u*l*r*d+
//            u*l*c*r*d+
//            u*l+
//            u*l*r+
//            u*l*c+
//            u*l*c*r+
                        
            // - end of added -
            0;
                        
            g[_j] = (c + f) % m;
                                    
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
