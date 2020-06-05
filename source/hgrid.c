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
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_vector(ns, p.nc);
        printf(" ---> %llu\n", hgrid[h]);
    }
    printf("\n");
}

void print_m2n2_hgrid(vector hgrid, struct parameters p) {
    printf("2,2 hg printer: unimplemented\n");
    
    element ns[p.nc];
    printf("(%llu,%llu): \n", p.m, p.n);
    for (nat h = 0; h < p.H; h++) {
        reduce(ns, h, p.m, p.nc);
        print_vector(ns, p.nc);
        printf(" ---> %llu\n", hgrid[h]);
    }
    printf("\n");
}

void print_m3n1_hgrid(vector hgrid, struct parameters p) {
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
    
    char line[2048] = {0};
    nat line_count = 0;
    
    while (fgets(line, sizeof line, file)) {
        line_count++;
        line[strlen(line) - 1] = '\0';
        if (line[0] == '#') continue;
            
    }
    fclose(file);
    printf("read %llu lines.\n", line_count);
}


void load_m3n1_hgrid(const char* filename, struct context* c) {
    
    printf("unimplemented!\n");
    
//    char path[2048] = {0};
//    strcpy(path, c->home);
//    strcat(path, filename);
//
//    FILE* file = fopen(path, "r");
//    if (!file) {
//        perror("fopen");
//        return;
//    }
//
//    char line[2048] = {0};
//    nat line_count = 0;
//
//    while (fgets(line, sizeof line, file)) {
//        line_count++;
//        line[strlen(line) - 1] = '\0';
//        if (line[0] == '#') continue;
//
//    }
//    fclose(file);
//    printf("read %llu lines.\n", line_count);
}
