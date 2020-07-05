/// current state of the ua theory.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>
typedef unsigned long long nat;

#define dot     f[s] = (f[s] + 1) % m
#define LRS(x) for (nat _ = 0; _ < x; _++)


nat unreduce(nat* in, nat radix, nat length) {
    nat s = 0;
    for (nat i = 0, p = 1; i < length; i++, p *= radix) s += p * in[i];
    return s;
}

void reduce(nat* out, nat s, nat radix, nat length) {
    for (nat i = 0, p = 1; i < length; i++, p *= radix) out[i] = (s / p) % radix;
}



int main(int argc, const char** argv) {
    if (argc <= 2) { printf("hgrid printer: 0: usage: \n\t./ca m n\n\n"); return 1; }
    
    const nat
        m = atoll(argv[1]),
        n = atoll(argv[2]),
        nc = 2 * n + 1,
        H = powl(m, nc);
    
    nat h[nc];
    
        
    printf("printing hgrid for builtin written equation for (%llu,%llu) ...\n", m, n);
    for (int k = 0; k < H; k++) {
        
        reduce(h, k, m, nc);
        const nat U = h[4], D = h[3], R = h[2], L = h[1], C = h[0];
        
        nat first = L * (C * (R + 1) + 1);
        nat composite = R * ((C + first) * (U + 1) + 1);
        nat second = U * ((C + first + composite) * (D + 1) + 1);
        
        const nat f = (C + first + composite + second) % m;
        
        printf("\n");
        printf("     %llu\n", h[3]);
        printf("   %llu %llu %llu --> %llu\n", h[1], h[0], h[2], f);
        printf("     %llu \n", h[4]);
        printf("\n");
        
        
//                puts(""); printf("    ");
//              fputs(h[3] ? "##" : "  ", stdout);
//              puts(""); printf("  ");
//              fputs(h[1] ? "##" : "  ", stdout);
//              fputs(h[0] ? "##" : "  ", stdout);
//              fputs(h[2] ? "##" : "  ", stdout);
//              printf("  -->  ");
//              fputs(f ? "##" : "  ", stdout);
//              puts(""); printf("    ");
//              fputs(h[4] ? "##" : "  ", stdout);
//              puts("\n");
        
        getchar();
    }
    
    printf("printed %llu rules.\n", H);
}
