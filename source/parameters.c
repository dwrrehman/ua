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

const char* stringify_initial_state(enum initial_state_type s) {
    if (s == empty_state) return "empty";
    if (s == dot_state) return "dot";
    if (s == center_dot_state) return "centerdot";
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
    printf("(%llu,%llu) : s=%llu:l=%llu,t=%llu : ", p.m, p.n, p.s, p.l, p.t);
    printf("[%s, %s] ", stringify_initial_state(p.initial_state), stringify_display_type(p.display_as));
    if (p.n_dimensional_display) printf("ND ");
    printf("delay=%llu \n", p.delay);
}

void verbose_print_parameters(struct parameters p) {
    printf("current parameters: \n");
    printf("\tm = %llu\n", p.m);
    printf("\tn = %llu\n", p.n);
    printf("\tcell count = %llu\n", p.s);
    printf("\tside length = %llu\n", p.l);
    printf("\ttimestep count = %llu\n", p.t);
    printf("\tdelay = %llu\n", p.delay);
    printf("\tinitial state type = %s : %d\n",stringify_initial_state(p.initial_state), p.initial_state);
    printf("\tn dimensional display = %d\n", p.n_dimensional_display);
    printf("\tdisplay type = %s : %d\n", stringify_display_type(p.display_as), p.display_as);
    printf("\n");
}

void set_parameter(struct parameters* p, const char* name, const char* value) {
    if (!name || !value) return;
    
    const nat v = atoll(value);
    
    if (equals(name, "m", "m")) p->m = v;
    else if (equals(name, "n", "n")) p->n = v;
    else if (equals(name, "s", "s")) p->s = v;
    else if (equals(name, "t", "t")) p->t = v;
    else if (equals(name, "l", "l")) p->l = v;
    else if (equals(name, "delay", "D")) p->delay = v;
    
    else if (strings_equal(name, "initial")) {
        if (equals(value, "empty", "e")) p->initial_state = empty_state;
        else if (equals(value, "dot", "d")) p->initial_state = dot_state;
        else if (strings_equal(value, "random")) p->initial_state = random_state;
        else if (strings_equal(value, "repeating")) p->initial_state = repeating_state;
        else if (equals(value, "centerdot", "c")) p->initial_state = center_dot_state;
        else printf("error: set_param: initial: unknown initial state value: %s\n", value);
        
    } else if (strings_equal(name, "display")) {
        if (strings_equal(value, "none")) p->display_as = no_display;
        else if (equals(value, "numeric", "n")) p->display_as = numeric_display;
        else if (equals(value, "intuitive", "i")) p->display_as = intuitive_display;
        else if (equals(value, "binary", "b")) p->display_as = binary_display;
        else printf("error: set_param: display: unknown display value: %s\n", value);
        
    } else if (strings_equal(name, "nd")) {
        if (equals(value, "false", "0")) p->n_dimensional_display = false;
        else if (equals(value, "true", "1")) p->n_dimensional_display = true;
        else printf("error: set_param: nd: must be a boolean: true or false, got: %s\n", value);
    }
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
}
