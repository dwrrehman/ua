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
    if (d == binary_display) return "binary";
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

void set_parameter(struct parameters* p, const char* name, const char* value) {
    if (!name || !value) return;
    
    const nat v = atoll(value);
    
    if (equals(name, "m", "m")) p->m = v;
    else if (equals(name, "n", "n")) p->n = v;
    else if (equals(name, "s", "s")) p->space = v;
    else if (equals(name, "t", "t")) p->time = v;
    else if (equals(name, "delay", "D")) p->delay = v;
    
    else if (strings_equal(name, "initial")) {
        if (strings_equal(value, "empty")) p->initial_state = empty_state;
        else if (strings_equal(value, "dot")) p->initial_state = dot_state;
        else if (strings_equal(value, "random")) p->initial_state = random_state;
        else if (strings_equal(value, "repeating")) p->initial_state = repeating_state;
        else printf("error: set_param: initial: unknown initial state value: %s\n", value);
        
    } else if (strings_equal(name, "display")) {
        if (strings_equal(value, "none")) p->display_as = no_display;
        else if (strings_equal(value, "numeric")) p->display_as = numeric_display;
        else if (strings_equal(value, "intuitive")) p->display_as = intuitive_display;
        else if (strings_equal(value, "binary")) p->display_as = binary_display;
        else printf("error: set_param: display: unknown display value: %s\n", value);
        
    } else if (strings_equal(name, "nd")) {
        if (strings_equal(value, "false")) p->n_dimensional_display = false;
        else if (strings_equal(value, "true")) p->n_dimensional_display = true;
        else printf("error: set_param: nd: must be a boolean: true or false, got: %s\n", value);
    }
    
    compute_derived_parameters(p);
}

void load_parameters_from_file(const char* filename, struct context* c) {
    
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
        set_parameter(&c->parameters, name, value);
    }
    
    fclose(file);
    printf("read %llu lines.\n", line_count);
    compute_derived_parameters(&c->parameters);
}
