// current state of the ua theory.
#include <unistd.h> // using:  usleep()
#include <stdio.h>  // using:  printf()
#include <stdlib.h> // using: atoll(), atoi()

// actually needed:
#include <string.h>
#include <math.h>

int main(int argc, const char** argv) {
    if (argc <= 5) { printf("usage: \n\t./ca m n l d t\n\n"); return 1; }
    const unsigned long long m = atoll(argv[1]), n = atoll(argv[2]), s = atoll(argv[3]), S = powl(s, n);
    const unsigned int delay = atoi(argv[4]), lifetime = atoi(argv[5]);
    unsigned long long g[S], h[S];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (unsigned long long i = 0; i < lifetime; i++) {
        if (i % 200 == 0 && i) printf("\e[1;1H\e[2J");
        memcpy(h, g, sizeof h);
        for (unsigned long long j = 0; j < S; j++) {
            
            const unsigned long long l = h[(j + s + 1) % s];
            const unsigned long long r = h[(j + s - 1) % s];
            const unsigned long long c = h[j];
            
            const unsigned long long e = c * (l * (l + 2) + 1);
            
            const unsigned long long d = c * (l + 1);
            
            const unsigned long long u = r * (e + d + 1);
            
            g[j] = (c + u) % m;
            
            printf("\033[38;5;%dm██\033[0m", (int)(24.0 * (double) h[j] / m) + 232);
        }
        printf("\n");
        usleep(delay);
    }
    return 0;
}
















//for (int f = 1; f < L; f *= l) {
//    g[j] += h[j + f * ((j / f + l + 1) % l - j / f % l)];
//    g[j] %= m;
//}

//            const long long left = h[(j + l - 1) % l];
