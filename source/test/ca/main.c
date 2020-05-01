/// current state of the ua theory.

#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll()

#include <string.h> // using memset(), memcpy().
#include <math.h>   // using powl()
typedef unsigned long long nat;

#define N for (nat i = 0; i < n; i++)
#define M for (nat j = 1; j < m; j++)

#define N2 for (nat k = 0; k < k; k++)
#define M2 for (nat l = 1; l < m; l++)

#define N3 for (nat _i = 0; _i < n; _i++)
#define M3 for (nat _j = 1; _j < m; _j++)

#define dot     f[s] = (f[s] + 1) % m

#define P(i)    for (nat _p = 0; _p < p[i]; _p++)
#define O       for (nat _o = 0; _o < o; _o++)
#define Q(k)    for (nat _q = 0; _q < q[k]; _q++)

int main(int argc, const char** argv) {
    if (argc <= 6) { printf("18: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        S = atoll(argv[3]), S_n = powl(S, n),
        T = atoll(argv[4]),
        delay = atoll(argv[5]),
        nd = atoll(argv[6]);
    
    nat f[S_n], g[S_n], p[n], q[n];
    
    memset(f, 0, sizeof f);
    if (n == 2) f[S_n / 2 + S / 2] = 1;
    else ++*f;
    
    for (nat t = 0; t < T; t++) {
        if (nd) printf("\e[1;1H\e[2J");
        memcpy(g, f, sizeof g);
        for (nat s = 0; s < S_n; s++) {
            
            nat y = 0;
            for (nat k = 1; k < S_n; k *= S) {
                p[y] = g[s + k * ((s / k + 1) % S - s / k % S)];
                q[y++] = g[s + k * ((s / k + S - 1) % S - s / k % S)];
            }
            
            for (nat i = 0; i < n; i++) {
                
                for (nat j = 1; j < m; j++) {
                                                            
                    for (nat _p = 0; _p < p[i]; _p++) {
                                                                        
                        for (nat _o = 0; _o < f[s]; _o++) {
                                                                                            
                            for (nat _q = 0; _q < q[i]; _q++) {
                                dot;
                            }
                            for (nat _k = 0; _k < j; _k++) {
                                dot;
                            }
                        }
                        dot;
                    }
                }
            }
        
            if (s % S == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) g[s] / m * 24) + 232);
            
        }
        printf("\n");
        fflush(stdout);
        usleep((unsigned) delay);
    }
}
