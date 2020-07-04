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
#include "ca.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void map(vector h, vector search, vector indicies, nat H) {
    for (nat i = 0, s = 0; i < H; i++)
        if (i == indicies[s]) h[i] = search[s++];
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

void print_n2_rule(vector h, nat f, enum display_type display_as, nat m) {
    if (display_as == no_display) return;
    else if (display_as == binary_display) {
        puts(""); printf("    ");
        fputs(h[3] ? "##" : "  ", stdout);
        puts(""); printf("  ");
        fputs(h[1] ? "##" : "  ", stdout);
        fputs(h[0] ? "##" : "  ", stdout);
        fputs(h[2] ? "##" : "  ", stdout);
        printf("  -->  ");
        if (f == unknown_dummy_value) printf("?"); else fputs(f ? "##" : "  ", stdout);
        puts(""); printf("    ");
        fputs(h[4] ? "##" : "  ", stdout);
        puts("\n");
        
    } else if (display_as == intuitive_display) {
        puts(""); printf("    ");
        graph(h[3], m);
        puts(""); printf("  ");
        graph(h[1],m); graph(h[0],m); graph(h[2],m);
        printf("  -->  ");
        if (f == unknown_dummy_value) printf("?"); else graph(f, m);
        puts(""); printf("    ");
        graph(h[4], m);
        puts("\n");
        
    } else if (display_as == numeric_display) {
        printf("\n");
        printf("     %llu\n", h[3]);
        printf("   %llu %llu %llu --> %llu\n", h[1], h[0], h[2], f);
        printf("     %llu \n", h[4]);
        printf("\n");
    }
}

void print_m2n2_hgrid(vector hgrid, struct parameters p) {
    if (!hgrid) { puts("{null hgrid}"); return; }
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
    printf("3,1 hg printer: unimplemented. printing as generic instead...\n");
    
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_vector(ns, p.nc);
        printf(" ---> %llu\n", hgrid[h]);
    }
    printf("\n");
}

void check_hgrid(struct context* c) {
    printf("checking file...\n");
    nat unknown_count = 0;
    for (nat i = 0; i < c->parameters.H; i++) {
        if (c->hgrid[i] == unknown_dummy_value) {
            printf("\nwarning: unspecified hgrid element: \n");
            element ns[c->parameters.nc];
            reduce(ns, i, c->parameters.m, c->parameters.nc);
            print_n2_rule(ns, unknown_dummy_value, c->parameters.display_as, c->parameters.m);
            unknown_count++;
        }
    }
    printf("checking complete: %llu unknowns.\n", unknown_count);
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
    nat line_count = 0, f = 0;
    element g[c->parameters.nc];
    
    destroy(&c->hgrid);
    c->hgrid = create(c->parameters.H);
    fill(unknown_dummy_value, c->hgrid, c->parameters.H);
            
    while (fgets(l, sizeof l, file)) {
        line_count++;
        l[strlen(l) - 1] = '\0';
        
        if (*l == '#') continue;
        
        else if (*l == '[') {
            assert(isdigit(l[6]) && "formatting incorrect in hgrid parse!");
            g[4] = l[6] - '0';
        } else if (*l == '|') {
            assert(isdigit(l[5]) &&
                   isdigit(l[6]) &&
                   isdigit(l[7]) &&
                   isdigit(l[13]) && "formatting incorrect in hgrid parse!");
            
            g[1] = l[5] - '0';
            g[0] = l[6] - '0';
            g[2] = l[7] - '0';
            f = l[13] - '0';
        }
        else if (*l == ']') {
            assert(isdigit(l[6]) && "formatting incorrect in hgrid parse!");
            g[3] = l[6] - '0';
            
        } else if (*l == '.') {
            c->hgrid[unreduce(g, c->parameters.m, c->parameters.nc)] = f;
            fill(0, g, c->parameters.nc);
        }
    }
    fclose(file);
    printf("read %llu lines.\n", line_count);
    check_hgrid(c);
}

void load_m3n1_hgrid(const char* filename, struct context* c) {
    printf("load_m3n1_hgrid: unimplemented!\n");
}

void load_hgrid(const char* filename, struct context* c) {
    printf("load_hgrid: unimplemented!\n");
}
