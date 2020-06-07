//
//  hgrid.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "hgrid.h"

#include "structures.h"
#include "vector.h"
#include "io.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void map(vector hgrid, vector known,
         vector known_indicies,
         vector search, nat H) {
    
    for (nat i = 0, j = 0, k = 0; i < H; i++)
        hgrid[i] = i == known_indicies[j]
            ? known[j++]
            : search[k++];
}

void print_hgrid(vector hgrid, struct parameters p) {
    if (!hgrid) { puts("{null hgrid}"); return; }
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_vector(ns, p.nc);
        printf(" ---> %llu\n", hgrid[h]);
    }
    printf("\n");
}

void graph(nat x, nat m) {
    printf("\033[38;5;%um██\033[0m", (unsigned)((double) x / m * 24) + 232);
}

void print_n2_rule(vector h, nat f, enum display_type display_as, nat m) {
    if (display_as == no_display) return;
    else if (display_as == intuitive_display) {
        puts("");
        printf("    ");
        graph(h[4], m);
        puts("");
        printf("  ");
        graph(h[2],m);
        graph(h[0],m);
        graph(h[1],m);
        printf("  -->  ");
        graph(f, m);
        puts("");
        printf("    ");
        graph(h[3], m);
        puts("\n");
        
    } else if (display_as == numeric_display) {
        printf("\n");
        printf("     %llu\n", h[4]);
        printf("   %llu %llu %llu --> %llu\n", h[2], h[0], h[1], f);
        printf("     %llu \n", h[3]);
        printf("\n");
    }
}

void print_m2n2_hgrid(vector hgrid, struct parameters p) {
    if (!hgrid) { puts("{null hgrid}"); return; }
    printf("2,2 hg printer: unimplemented\n");
    
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_n2_rule(ns, hgrid[h], p.display_as, p.m);
    }
    printf("\n");
}

void print_m3n1_hgrid(vector hgrid, struct parameters p) {
    if (!hgrid) { puts("{null hgrid}"); return; }
    printf("3,1 hg printer: unimplemented\n");
    
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_vector(ns, p.nc);
        printf(" ---> %llu\n", hgrid[h]);
    }
    printf("\n");
}

void load_m2n2_hgrid(const char* filename, struct context* c) {
    
    char path[2048] = {0};
    strcpy(path, c->home);
    strcat(path, filename);
    
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("fopen");
        return;
    }
    
    char l[2048] = {0};
    nat line_count = 0;
    
    element g[c->parameters.nc];
    nat f = 0;
    
    while (fgets(l, sizeof l, file)) {
        line_count++;
        l[strlen(l) - 1] = '\0';
        if (*l == '#') continue;
        else if (*l == '[') g[4] = l[6] - '0';
        else if (*l == '|') { g[2] = l[5] - '0'; g[0] = l[6] - '0'; g[1] = l[7] - '0'; f = l[13] - '0'; }
        else if (*l == ']') g[3] = l[6] - '0';        
        else if (*l == '.') {
            printf("just read rule: ");
            print_vector(g, 5);
            printf(" ----> %llu\n\n", f);
        }
        
    }
    fclose(file);
    printf("read %llu lines.\n", line_count);
}


void load_m3n1_hgrid(const char* filename, struct context* c) {
    printf("load_m3n1_hgrid: unimplemented!\n");
}

void load_hgrid(const char* filename, struct context* c) {
    printf("load_hgrid: unimplemented!\n");
}
