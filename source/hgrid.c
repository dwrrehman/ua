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
#include <math.h>
//
//nat compute_H_p(struct parameters p) {
//    return powl(p.m,p.n);
//}
//
//nat compute_H_c(struct context context) {
//    return powl(context.parameters.m,context.parameters.n);
//}

nat to(nat base, nat exponent) {
    return powl(base, exponent);
}

void map(vector h, vector search, vector indicies, nat H) {
    for (nat i = 0, s = 0; i < H; i++)
        if (i == indicies[s]) h[i] = search[s++];
}

void print_n5_rule(vector h, nat f, enum display_type display_as, nat m) {
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


void print_n4_rule(vector h, nat f, enum display_type display_as, nat m) {
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
        puts("\n");
        
    } else if (display_as == intuitive_display) {
        puts(""); printf("    ");
        graph(h[3], m);
        puts(""); printf("  ");
        graph(h[1],m); graph(h[0],m); graph(h[2],m);
        printf("  -->  ");
        if (f == unknown_dummy_value) printf("?"); else graph(f, m);
        puts("\n");
        
    } else if (display_as == numeric_display) {
        printf("\n");
        printf("     %llu\n", h[3]);
        printf("   %llu %llu %llu --> %llu\n", h[1], h[0], h[2], f);
        printf("\n");
    }
}


void print_n3_rule(vector h, nat f, enum display_type display_as, nat m) {
    if (display_as == no_display) return;
    else if (display_as == binary_display) {
        printf("\n   ");
        fputs(h[1] ? "##" : "  ", stdout);
        fputs(h[0] ? "##" : "  ", stdout);
        fputs(h[2] ? "##" : "  ", stdout);
        printf("  -->  ");
        if (f == unknown_dummy_value) printf("?"); else fputs(f ? "##" : "  ", stdout);
        puts("\n");
        
    } else if (display_as == intuitive_display) {
        printf("\n   ");
        graph(h[1],m); graph(h[0],m); graph(h[2],m);
        printf("  -->  ");
        if (f == unknown_dummy_value) printf("?"); else graph(f, m);
        puts("\n");
        
    } else if (display_as == numeric_display) {
        printf("\n");
        printf("   %llu %llu %llu --> %llu\n", h[1], h[0], h[2], f);
        printf("\n");
    }
}

void print_hgrid(vector hgrid, struct parameters parameters, bool generic) {
    if (!hgrid) { puts("{null hgrid}"); return; }
    
    const nat m = parameters.m, n = parameters.n, H = to(m,n);
    
    element neighborhood[n];
    
    printf("(%llu,%llu): \n", m, n);
    
    for (nat rule = 0; rule < H; rule++) {
        reduce(neighborhood, rule, m, n);
        if (generic) {
            print_vector(neighborhood, n);
            printf(" ---> %llu\n", hgrid[H]);
        } else {
            if (n == 3) print_n3_rule(neighborhood, hgrid[H], parameters.display_as, m);
            if (n == 4) print_n4_rule(neighborhood, hgrid[H], parameters.display_as, m);
            if (n == 5) print_n5_rule(neighborhood, hgrid[H], parameters.display_as, m);
        }
    }
    printf("\n");
}


void check_hgrid(struct context* context) {
    printf("checking hgrid file...\n");
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    nat unknown_count = 0;
    for (nat i = 0; i < H; i++) {
        if (context->hgrid[i] == unknown_dummy_value) {
            printf("\nwarning: unspecified hgrid element: \n");
            element neighborhood[n];
            reduce(neighborhood, i, m, n);
            print_n5_rule(neighborhood, unknown_dummy_value, context->parameters.display_as, m);
            unknown_count++;
        }
    }
    printf("checking complete: %llu unknowns.\n", unknown_count);
}

void load_n5_hgrid(const char* filename, struct context* context) {
    
    // works for m <= 10.
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    assert(n == 5);
        
    char path[2048] = {0};
    strcpy(path, context->home);
    strcat(path, filename);
    
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("fopen");
        return;
    }
    
    char line[2048] = {0};
    nat line_count = 0, future = 0;
    element neighborhood[n];
    
    destroy(&context->hgrid);
    context->hgrid = create(H);
    fill(unknown_dummy_value, context->hgrid, H);
            
    while (fgets(line, sizeof line, file)) {
        line_count++;
        line[strlen(line) - 1] = '\0';
        
        if (*line == '#') continue;
        
        else if (*line == '[') {
            assert(isdigit(line[6]) && "formatting incorrect in hgrid parse!");
            neighborhood[3] = line[6] - '0';
            
        } else if (*line == '|') {
            assert(isdigit(line[5]) &&
                   isdigit(line[6]) &&
                   isdigit(line[7]) &&
                   isdigit(line[13]) && "formatting incorrect in hgrid parse!");
            
            neighborhood[1] = line[5] - '0';
            neighborhood[0] = line[6] - '0';
            neighborhood[2] = line[7] - '0';
            future = line[13] - '0';
        }
        else if (*line == ']') {
            assert(isdigit(line[6]) && "formatting incorrect in hgrid parse!");
            neighborhood[4] = line[6] - '0';
            
        } else if (*line == '.') {
            context->hgrid[unreduce(neighborhood, m, n)] = future;
            fill(0, neighborhood, n);
        } else if (*line == '?') {
            context->hgrid[unreduce(neighborhood, m, n)] = unknown_dummy_value;
            fill(0, neighborhood, n);
        }
    }
    fclose(file);
    printf("read %llu lines.\n", line_count);
    check_hgrid(context);
}

void load_n3_hgrid(const char* filename, struct context* c) {
    printf("load_n3_hgrid: unimplemented!\n");
}

void load_n4_hgrid(const char* filename, struct context* c) {
    printf("load_n4_hgrid: unimplemented!\n");
}


void filter_using_blacklist(char** input) {
    printf("filter: filtering z values not in blacklist...\n");
    nat z_count = 0, bl_count = 0, count = 0;
    vector
    z_values = read_nats_from_file(input[2], &z_count),
    blacklist = read_nats_from_file(input[3], &bl_count),
    out = create(z_count);
    
    for (nat i = 0; i < z_count; i++) {
        bool is_blacklisted = false;
        for (nat j = 0; j < bl_count; j++) if (z_values[i] == blacklist[j]) is_blacklisted = true;
        if (!is_blacklisted) out[count++] = z_values[i];
    }
    
    printf("filter: writing %llu filtered z values...\n", count);
    write_nats_to_file(input[4], out, count);
    destroy(&out);
}
