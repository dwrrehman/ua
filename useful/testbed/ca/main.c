/// current state of the ua theory.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

typedef unsigned long long nat;

#define dot     f[s] = (f[s] + 1) % m
#define LRS(x) for (nat _ = 0; _ < x; _++)

int main(int argc, const char** argv) {
    if (argc <= 6) { printf("new: 23: usage: \n\t./ca m n s t d nd\n\n"); return 1; }
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        s = atoll(argv[3]), S = powl(s, n / 2),
        t = atoll(argv[4]),
        delay = atoll(argv[5]),
        nd = atoll(argv[6]);
    
    nat f[S], g[S], h[n]; // excluding C_0.
    memset(f, 0, sizeof f); ++*f;    // construct dot state.
    
    for (nat _t = 0; _t < t; _t++) {
        if (nd) printf("\e[1;1H\e[2J");
        memcpy(g, f, sizeof g);
        for (nat _s = 0; _s < S; _s++) {
            
            nat y = 0;
            h[y++] = g[_s];
            for (nat x = 1; x < S; x *= s) {
                h[y++] = g[_s + x * ((_s / x + s + 1) % s - _s / x % s)];
                h[y++] = g[_s + x * ((_s / x + s - 1) % s - _s / x % s)];
            }
            
            for (nat i = 1; i < n; i++) {
                f[_s] += h[i] * (f[_s] * (h[i - 1] + 1) + 1);
            }
            
            
            f[_s] %= m;
            if (_s % s == 0 && nd) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) g[_s] / m * 24) + 232);
        }
        printf("\n");
        fflush(stdout);
        if (!delay) getchar(); else usleep((unsigned) delay);
    }
}
