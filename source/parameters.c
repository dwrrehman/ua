//
//  parameters.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "parameters.h"

#include "structures.h"
#include "io.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

bool is_mn_case(nat m, nat n, struct parameters p) { return p.m == m && p.n == n; }

void compute_derived_parameters(struct parameters* p) {
    p->nc = 2 * p->n + 1;
    p->H = powl(p->m, p->nc);
    p->L = powl(p->space, p->n);    
}

const char* stringify_initial_state(enum initial_state_type s) {
    if (s == empty_state) return "empty";
    if (s == dot_state) return "dot";
    if (s == random_state) return "random";
    if (s == repeating_state) return "repeating";
    return "unknown state";
}

const char* stringify_display_type(enum display_type d) {
    if (d == no_display) return "no display";
    if (d == numeric_display) return "numeric";
    if (d == intuitive_display) return "intuitive";
    return "unknown display";
}


void print_parameters(struct parameters p) { 
    printf("(%llu,%llu : %llu,%llu) ", p.m, p.n, p.space, p.time);
    printf("[%s, %s] ", stringify_initial_state(p.initial_state), stringify_display_type(p.display_as));
    if (p.n_dimensional_display) printf("ND ");
    printf("delay=%llu ", p.delay);
    printf("| nc=%llu H=%llu L=%llu", p.nc, p.H, p.L);
    printf("\n");
}

void verbose_print_parameters(struct parameters p) {
    printf("current parameters: \n");
    printf("\tm = %llu\n", p.m);
    printf("\tn = %llu\n", p.n);
    printf("\tspace = %llu\n", p.space);
    printf("\ttime = %llu\n", p.time);
    printf("\tdelay = %llu\n", p.delay);
    printf("\tinitial_state_type = %d\n", p.initial_state);
    printf("\tn_dimensional_display = %d\n", p.n_dimensional_display);
    printf("\tdisplay_type = %d\n", p.display_as);
        
    printf("computed parameters:\n");
    printf("\tnc = %llu\n", p.nc);
    printf("\tH = %llu\n", p.H);
    printf("\tL = %llu\n", p.L);
    printf("\n");
}

void load_parameters(const char* filename, struct context* c) {
    
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
        char name[128] = {0}, value[128] = {0};
        sscanf(line, "%s = %s", name, value);
                 
        if (strings_equal(name, "m")) c->parameters.m = atoll(value);
        else if (strings_equal(name, "n")) c->parameters.n = atoll(value);
        else if (strings_equal(name, "s")) c->parameters.space = atoll(value);
        else if (strings_equal(name, "t")) c->parameters.time = atoll(value);
        else if (strings_equal(name, "delay")) c->parameters.delay = atoll(value);
        
        else if (strings_equal(name, "initial")) {
            if (strings_equal(value, "empty")) c->parameters.initial_state = empty_state;
            else if (strings_equal(value, "dot")) c->parameters.initial_state = dot_state;
            else if (strings_equal(value, "random")) c->parameters.initial_state = random_state;
            else if (strings_equal(value, "repeating")) c->parameters.initial_state = repeating_state;
            else printf("error: load: param: initial: line %llu: error in file \n", line_count);
        }
        else if (strings_equal(name, "display")) {
            if (strings_equal(value, "none")) c->parameters.display_as = no_display;
            else if (strings_equal(value, "numeric")) c->parameters.display_as = numeric_display;
            else if (strings_equal(value, "intuitive")) c->parameters.display_as = intuitive_display;
            else printf("error: load: param: display: line %llu: error in file \n", line_count);
        }
        else if (strings_equal(name, "nd")) {
            if (strings_equal(value, "false")) c->parameters.n_dimensional_display = false;
            else if (strings_equal(value, "true")) c->parameters.n_dimensional_display = true;
            else printf("error: load: param: nd: line %llu: error in file \n", line_count);
        }
    }
    fclose(file);
    printf("read %llu lines.\n", line_count);
    compute_derived_parameters(&c->parameters);
}
