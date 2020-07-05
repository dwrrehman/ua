#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void print(size_t* h, size_t l, size_t m, size_t n, bool n_dimensional_display, bool numeric_print) {
    if (n_dimensional_display) printf("\e[1;1H\e[2J");
    for (size_t i = 0; i < l; i++) {
        if (numeric_print) printf("%4lu ", h[i]);
        else printf("\033[38;5;%lum██\033[0m", 232 + (size_t) (24.0 * ((double) h[i] / (double) m)));
        if (n_dimensional_display) {
            for (size_t q = 1; q < n; q++) {
                size_t d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");    
}

int main(void) {
    size_t m = 10, n = 1, l = pow(m, n), g[l], h[l];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (size_t i = l; i--;) {
        memcpy(h, g, sizeof h);
        for (size_t j = l; j--;) {
            for (size_t k = 1; k < l; k *= m) {
                g[j] = 0;                                       // unfinished
            }
        }
        print(h, l, m, n, 0, 1);
        usleep(100000);
    }
    return 0;
}
