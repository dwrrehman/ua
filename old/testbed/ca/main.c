/// current state of the ua theory.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>
typedef unsigned long long nat;

#define dot     f[s] = (f[s] + 1) % m
#define L(x) for (nat _ = 0; _ < x; _++)

int main(int argc, const char** argv) {
    if (argc <= 6) { printf("new: 19: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
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
                p[y] = g[s + k * ((s / k + S + 1) % S - s / k % S)];
                q[y] = g[s + k * ((s / k + S - 1) % S - s / k % S)];
                y++;
            }
            const nat U = p[1], D = q[1], R = p[0], L = q[0], C = g[s];
            
            
            
            f[s] =
             (
              // RL + LC + LU + CU + RLC + RLU + LCU + L + C + U
              R*L + L*C + L*U + C*U + R*L*C + R*L*U + L*C*U + L + C + U

             ) % m;
            
            if (s % S == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) g[s] / m * 24) + 232);
            
        }
        printf("\n");
        fflush(stdout);
        usleep((unsigned) delay);
    }
}
