//
//  ca.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "ca.h"

#include "structures.h"
#include "vector.h"
#include "io.h"
#include "hgrid.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const nat unknown_dummy_value = 999;

char mode = stopped;




void handler(int e) {    
    printf("interrupt(q/r/x/p) ");
    int c = get_character();
    if (c == 'q') mode = stopped;
    if (c == 'r') mode = running;
    if (c == 'p') mode = paused;
    if (c == 'x') exit(0);
}

void graph(nat x, nat m) {
    printf("\033[38;5;%um██\033[0m", (unsigned)((double) x / m * 24) + 232);
}

nat find_unknown_indicies(vector hg, vector I, nat H) {
    nat count = 0;
    for (nat i = 0; i < H; i++)
        if (hg[i] == unknown_dummy_value) I[count++] = i;
    return count;
}




void initialize(vector g, nat m, nat L,
                enum initial_state_type initial) {
    fill(0, g, L);
    if (initial == empty_state) return;
    else if (initial == dot_state) g[0] = 1;
    else if (initial == repeating_state) {
        for (nat i = 0; i < L; i++) g[i] = i % 2 == 0;
    } else if (initial == random_state) {
        for (nat i = 0; i < L; i++) g[i] = rand() % m;
    }
}

void fill_neighbors(vector read_array, nat cell,
                    vector neighbors, nat L, nat space) {
    neighbors[0] = read_array[cell];
    nat y = 1;
    for (nat f = 1; f < L; f *= space) {
        neighbors[y++] = read_array
        [cell + f * ((cell / f + 1) % space
                     - cell / f % space)]; // R
        neighbors[y++] = read_array
        [cell + f * ((cell / f + space - 1) % space
                     - cell / f % space)]; // L
    }
}

nat measure_lifetime_for_hgrid(vector h, struct parameters* p) {
    
    const nat
        S = p->L,
        m = p->m,
        nc = p->nc,
        space = p->space,
        time = p->time;
    
    nat count = 0;
    element ns[nc], f[S], g[S], states[time];
    initialize(f, m, S, p->initial_state);
    
    for (nat t = 0; t < time; t++) {
        memcpy(g, f, sizeof g);
        for (nat s = 0; s < S; s++) {
            fill_neighbors(g, s, ns, S, space);
            f[s] = h[unreduce(ns, m, nc)];
            const nat r = unreduce(g, m, S);
            if (!contains(states, count, r))
                states[count++] = r;
        }
    }
    return count;
}

void visualize_lifetime(vector h, struct parameters* p) {
    
    signal(SIGINT, handler);
    
    const nat
        S = p->L,
        m = p->m,
        nc = p->nc,
        space = p->space,
        time = p->time;
    
    mode = running;
        
    element ns[nc], f[S], g[S];
    initialize(f, m, S, p->initial_state);
    
    for (nat t = 0; t < time && mode != stopped; t++) {
        while (mode == paused) sleep(1);
        
        memcpy(g, f, sizeof g);
        for (nat s = 0; s < S; s++) {
            
            fill_neighbors(g, s, ns, S, space);
            f[s] = h[unreduce(ns, m, nc)];
            
            if (p->display_as == intuitive_display) graph(g[s], m);
            else if (p->display_as == numeric_display) printf(" %llu", g[s]);
            if (s % space == 0 && s && p->n_dimensional_display) puts("");
        }
        puts("");
        fflush(stdout);
        usleep((unsigned) p->delay);
    }
    mode = stopped;
}


void save_z_values(const char* out, vector definitions, nat count) {
    printf("would you like to write the %llu z values to the file \"%s\"? (y/n) ", count, out);
    int response = get_character();
    if (response != 'y') return;
    
    printf("writing %llu z values to %s...\n", count, out);
    write_nats_to_file(out, definitions, count);
    printf("done.\n");
}

void threshold_search(nat threshold, const char* outfile, struct context* c) {
    
    const nat
        m = c->parameters.m,
        H = c->parameters.H;
    
    vector hg = duplicate(c->hgrid, H);
    element indicies[H], search[H];
    
    const nat
        u = find_unknown_indicies(hg, indicies, H),
        Z = powl(m, u);
    
    printf("searching over %llu unknowns...\n", u);

    element definitions[Z], scores[Z];
    nat count = 0;
    
    mode = running;
    
    for (nat z = 0; z < Z && mode != stopped; z++) {
        while (mode == paused) sleep(1);
        
        printf("\r [  %llu  /  %llu  ]       ", z, Z);
        reduce(search, z, m, u);
        map(hg, search, indicies, H);
        
        const nat
            score = measure_lifetime_for_hgrid(hg, &c->parameters),
            definition = unreduce(hg, m, H);
        
        if (score >= threshold) {
             printf("\n[z = %llu] ---> %llu timesteps\n\n", definition, score);
            definitions[count] = definition;
            scores[count++] = score;
        }
    }
    
    puts("\n");
    printf("found %llu z values above threshold. (%f%%) \n", count, ((float)count / Z));
    save_z_values(outfile, definitions, count);
    
    mode = stopped;
}
