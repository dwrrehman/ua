#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc, const char** argv) {
    if (argc <= 5) { printf("usage: \n\t./ca m n s t d nd\n\n"); return 1; }
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
        memcpy(h, g, sizeof h);
        for (unsigned long long j = 0; j < S; j++) {
            const unsigned long long c = h[j];
            const unsigned long long r = h[(j + 1) % s];
            const unsigned long long l = h[(j + s - 1) % s];
            g[j] = (g[j] + r * (c * (l + 1) + 1)) % m;
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) h[j] / m * 24) + 232);
            fflush(stdout);
        }
        printf("\n");
        usleep((unsigned) d);
    }
    return 0;
}
