// ua code:
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define and &&
#define or ||
#define not !
typedef long nat;

void print_pretty(nat* h, nat l) {
    for (nat i = 0; i < l; i++) {
        printf("%s", h[i] == 0 ? "  " : "##");
    } printf("\n");
}

void print(nat* h, nat l) {
    for (nat i = 0; i < l; i++) {
        printf("%5ld", h[i]);
    } printf("\n");
}

static nat sum(nat *h, nat j, nat n, nat size, nat L) {
    nat total = h[j];
    for (int f = 1; f < L; f *= size) {
        total += h[j + f * ((j / f + 1) % size - j / f % size)] + h[j + f * ((j / f + size - 1) % size - j / f % size)];
    }
    return total;
}

static void fill_neighbors(nat* h, nat j, nat* ns, const nat L, const nat size) {
    ns[0] = h[j];
    nat y = 1;
    for (int f = 1; f < L; f *= size) {
        ns[y++] = h[j + f * ((j / f + 1) % size - j / f % size)];
        ns[y++] = h[j + f * ((j / f + size - 1) % size - j / f % size)];
    }
}

int main(int argc, const char** argv) {
    if (argc < 4) { printf("./rule110 size lifetime delay\n"); exit(1); }
    
    const nat m = 2, n = 1,
    size = atol(argv[1]),
    lifetime = atol(argv[2]),
    delay = atoi(argv[3]);
    
    nat L = pow(size, n);
    nat g[L], h[L];
    
    memset(g, 0, sizeof g);
    g[0] = 1;
    
    for (nat i = 0; i < lifetime; i++) {
        memcpy(h, g, sizeof h);
        for (nat j = 0; j < size; j++) {
            nat ns[2 * n + 1];
            fill_neighbors(h, j, ns, L, size);
            nat total = sum(h, j, n, size, L);
            g[j] = total >= 1 and total <= 2 and not (ns[0] == 0 and ns[1] == 1 and ns[2] == 0);
        }
        print_pretty(h, size);
        usleep((unsigned) delay);
    }
    
    return 0;
}
