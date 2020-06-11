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
    printf("interrupt(q/r/X) ");
    fflush(stdout);
    const int c = get_character();
    if (c == 'X') exit(0);
    else if (c == 'q') mode = stopped;
    else if (c == 'r') mode = running;
    printf("\n");
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


void initialize(vector g, nat m, nat n, nat L, nat s,
                enum initial_state_type initial) {
    fill(0, g, L);
    if (initial == empty_state) return;
    else if (initial == dot_state) {
        
        if (n == 2) g[L / 2 + s / 2] = 1;
        else ++*g;
    }
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

nat measure_lifetime(vector h, struct parameters* p) {
    
    const nat
        S = p->L,
        m = p->m,
        n = p->n,
        nc = p->nc,
        space = p->space,
        time = p->time;
    
    nat count = 0;
    element ns[nc], f[S], g[S], states[time];
    initialize(f, m, n, S, space, p->initial_state);
    
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

void visualize_lifetime(nat begin, nat begin_slice, nat end_slice, vector h, struct parameters p) {
    const nat S = p.L, m = p.m, n = p.n, nc = p.nc, space = p.space, time = p.time;
    mode = running;
    element ns[nc], f[S], g[S];
    initialize(f, m, n, S, space, p.initial_state);
    
    for (nat t = 0; t < begin + time && mode != stopped; t++) {
        if (p.n_dimensional_display) clear_screen();
        
        memcpy(g, f, sizeof g);
        
        for (nat s = 0; s < S; s++) {
            fill_neighbors(g, s, ns, S, space);
            f[s] = h[unreduce(ns, m, nc)];

            const nat slice = s / space;
            if (t >= begin &&
                ((begin_slice <= slice && slice < end_slice)
                 || (!end_slice && !begin_slice))
                ) {
                if (p.display_as == intuitive_display) graph(g[s], m);
                else if (p.display_as == numeric_display) printf(" %llu", g[s]);
                else if (p.display_as == binary_display) fputs(g[s] ? "##" : "  ", stdout);
                
                if (s % space == 0 && s && p.n_dimensional_display) puts("");
            }
            
        }
        
        if (t >= begin) {
            puts("");
            if (p.delay) fflush(stdout);
            usleep((unsigned) p.delay);
        }
    }
}

void save_values(const char* out_filename, vector values, nat count) {
    while (true) {
        printf("would you like to write the %llu values to the file \"%s\"? (yes/no) ", count, out_filename);
        fflush(stdout);
        
        char response[128] = {0};
        fgets(response, sizeof response, stdin);
        
        if (strings_equal(response, "yes\n")) {
            printf("writing %llu values to %s...\n", count, out_filename);
            write_nats_to_file(out_filename, values, count);
            printf("save complete.\n");
            return;
            
        } else if (strings_equal(response, "no\n")) {
            printf("save aborted, values discarded.\n");
            return;
        } else
            printf("error: save: invalid response. please type \"yes\" or \"no\".\n");
    }
}

void threshold_search(nat threshold, const char* outfile, struct context* c) {
    
    nat m = c->parameters.m, H = c->parameters.H;
    vector hg = duplicate(c->hgrid, H);
    element indicies[H], search[H];
    nat u = find_unknown_indicies(hg, indicies, H), Z = powl(m, u), count = 0;
    element definitions[Z], scores[Z];
    mode = running;
    
    printf("searching over %llu unknowns...\n", u);
    for (nat z = 0; z < Z && mode != stopped; z++) {
        printf("\r [  %llu  /  %llu  ]       ", z, Z);
        reduce(search, z, m, u);
        map(hg, search, indicies, H);
        nat score = measure_lifetime(hg, &c->parameters), definition = unreduce(hg, m, H);
        if (score >= threshold) {
             printf("\n[z = %llu] ---> %llu timesteps\n\n", definition, score);
            definitions[count] = definition;
            scores[count++] = score;
        }
    }
    
    puts("\n");
    printf("found %llu z values above threshold. (%f%%) \n", count, ((float)count / Z));
    save_values(outfile, definitions, count);
}

void visualize_set(nat begin, vector set, nat count,
                   const char* savelist_out_filename,
                   const char* blacklist_out_filename,
                   struct context* context) {
        
    nat offset = 0, at = begin, begin_slice = 0, end_slice = 1;
    element blacklist[count], savelist[count], hgrid[context->parameters.H];
    fill(0, blacklist, count); fill(0, savelist, count);
    
    while (true) {
        clear_screen();
        
        if (at < count) {
            nat z = set[at];
            reduce(hgrid, z, context->parameters.m, context->parameters.H);
            nat d = context->parameters.delay; context->parameters.delay = 0;
            visualize_lifetime(offset, begin_slice, end_slice, hgrid, context->parameters);
            context->parameters.delay = d;
            
            printf("\n  [z = %llu]  %llu  /  %llu  ", z, at, count);
            if (blacklist[at]) printf("(blacklisted) ");
            if (savelist[at]) printf("(saved) ");
            printf(":> ");
        } else printf("\n  [END]  %llu  /  %llu  :> ", at, count);
        

        
        fflush(stdout);
        const int c = get_character();
        
        if (c == 'd') offset += context->parameters.time;
        else if (c == 's' && offset >= context->parameters.time) offset -= context->parameters.time;
        
        else if (c == 'f' && at < count) { at++; offset = 0; }
        else if (c == 'e' && at) { at--; offset = 0; }
        
        else if (c == 'w') savelist[at] = !savelist[at];
        else if (c == 'a') blacklist[at] = !blacklist[at];
        
        else if (c == 'j' && begin_slice) begin_slice--;
        else if (c == 'i' && begin_slice + 1 < end_slice) begin_slice++;
        else if (c == 'o' && end_slice - 1 > begin_slice) end_slice--;
        else if (c == ';') end_slice++;
        
        else if (c == 'g') user_interface(context);
        else if (c == 'Q') break;
        
    }
    
    element saved[count], blacklisted[count];
    nat saved_count = 0, blacklisted_count = 0;
    for (nat i = 0; i < count; i++) {
        if (savelist[i])
            saved[saved_count++] = set[i];
        
        if (blacklist[i])
            blacklisted[blacklisted_count++] = set[i];
    }
    
    save_values(savelist_out_filename, saved, saved_count);
    save_values(blacklist_out_filename, blacklisted, blacklisted_count);
}
