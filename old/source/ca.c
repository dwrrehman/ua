
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
#include <errno.h>

const nat unknown_dummy_value = 999;

char mode = stopped;

void handler(int e) {
    printf("modes: \n"
           "\t X = kill process\n"
           "\t q = stop command\n"
           "\t r = resume/running\n"
           "\t s = step thru\n"
           );
    printf("interrupt(q/r/X/s) ");
    fflush(stdout);
    const int c = get_character();
    if (c == 'X') exit(0);
    else if (c == 'q') mode = stopped;
    else if (c == 'r') mode = running;
    else if (c == 's') mode = stepping;
    printf("\n");
}

void graph(nat x, nat m) {
    printf("\033[38;5;%um██\033[0m", (unsigned)((double) x / m * 24) + 232);
}

void generate_ppm(const char* filename, const int* g, const int m, const int s, const int L) {
    FILE* file = fopen(filename, "wb");
    fprintf(file, "P6\n%d %d\n255\n", s, s);
    for (int cell = 0; cell < L; cell++) {
        double x = (double) g[cell] / (double) m;
        unsigned char r = x * 255, g = x * 255, b = x * 255;
        fwrite(&r, 1, 1, file);
        fwrite(&g, 1, 1, file);
        fwrite(&b, 1, 1, file);
    }
    fclose(file);
}


nat find_unknown_indicies(vector hgrid, nat hgrid_count, vector indicies) {
    nat count = 0;
    for (nat i = 0; i < hgrid_count; i++)
        if (hgrid[i] == unknown_dummy_value) indicies[count++] = i;
    return count;
}

void initialize(vector cells, nat m, nat n,
                nat cell_count, nat sidelength,
                enum initial_state_type initial) {
    fill(0, cells, cell_count);
    if (initial == empty_state) return;
    else if (initial == dot_state) ++*cells;
    else if (initial == center_dot_state) {
        if (n == 5) cells[cell_count / 2 + (sidelength / 2) /* * (s % 2 == 0)*/] = 1;
        else if (n == 4) cells[cell_count / 2] = 1;
        else if (n == 3) cells[cell_count / 2] = 1;
        else ++*cells;
    } else if (initial == repeating_state) {
        for (nat i = 0; i < cell_count; i++) cells[i] = i % m;
    } else if (initial == random_state) {
        for (nat i = 0; i < cell_count; i++) cells[i] = rand() % m;
    }
}

void fill_balanced_neighbors(nat n, /*input*/vector read_array, nat cell,
                    /*output*/vector neighbors, nat cell_count, nat sidelength) {
    
    /// fills the neighborhood according to the natural ordering:
    ///
    ///     h(x)  =  { C L R U D F B A P ...etc }
    ///
    /// uses an algorithm as follows:
    ///
    /// the index for the positive and negative
    /// neighbor along the i / l dimension, for the cell index "c":
    ///
    ///     c + i * ((c / i + l ± 1) % l - c / i % l
    ///
    /// this would then be indexed into the read array,
    /// to get the neighbors.
    
    nat count = 0;
    
    if (count < n) neighbors[count++] = read_array[cell];
    
    if (n == 3) {
        
        if (count < n) neighbors[count++] = read_array
            [cell + ((cell + sidelength - 1) % sidelength - cell % sidelength)];
        
        if (count < n) neighbors[count++] = read_array
            [cell + ((cell + 1) % sidelength - cell % sidelength)];
        
    } else if (n == 4) {
        
        if (count < n) neighbors[count++] = read_array
            [cell + ((cell + sidelength - 1) % sidelength - cell % sidelength)];
        
        if (count < n) neighbors[count++] = read_array
            [cell + ((cell + 1) % sidelength - cell % sidelength)];
        
        if (count < n) neighbors[count++] = read_array
            [cell + ((cell + 2) % sidelength - cell % sidelength)];
        
    } else if ( n == 5) {
        for (nat i = 1; i < cell_count; i *= sidelength) {
            if (count < n) neighbors[count++] = read_array [cell + i * ((cell / i + sidelength - 1) % sidelength - cell / i % sidelength)];
            if (count < n) neighbors[count++] = read_array [cell + i * ((cell / i + 1) % sidelength - cell / i % sidelength)];
        }
    } else
        abort(); // lol :P
}

bool doesnt_contain_vector(nat element_length, vector* states, nat state_count, vector state) {
    for (nat i = 0; i < state_count; i++)
        if (vectors_equal(states[i], element_length, state, element_length))
            return false;
    return true;
}

nat measure_lifetime(vector hgrid, struct parameters* p) {
    
    const nat m = p->m, n = p->n, cell_count = p->s, timesteps = p->t, sidelength = p->l;
    
    element neighborhood[n], write_cells[cell_count], read_cells[cell_count];
    
    nat state_count = 0;
    vector states[timesteps];
    
    initialize(write_cells, m, n, cell_count, sidelength, p->initial_state);
    
    for (nat timestep = 0; timestep < timesteps; timestep++) {
        
        memcpy(read_cells, write_cells, sizeof read_cells);
        
        for (nat cell = 0; cell < cell_count; cell++) {
            fill_balanced_neighbors(n, read_cells, cell, neighborhood, cell_count, sidelength);
            write_cells[cell] = hgrid[unreduce(neighborhood, m, n)];
        }
        
        if (doesnt_contain_vector(cell_count, states, state_count, read_cells))
            states[state_count++] = duplicate(read_cells, cell_count);
        else break;
    }
    
    for (nat i = 0; i < state_count; i++) destroy(states + i);
    return state_count;
}

void visualize_lifetime(nat begin_timestep, nat begin_slice, nat end_slice, vector hgrid, struct parameters p) {
    
    const nat m = p.m, n = p.n, cell_count = p.s, timesteps = p.t, sidelength = p.l;
    mode = running;
    
    element
        neighborhood[n],
        write_cells[cell_count],
        read_cells[cell_count];
    
    initialize(write_cells, m, n, cell_count, sidelength, p.initial_state);
    
    for (nat timestep = 0; timestep < begin_timestep + timesteps && mode != stopped; timestep++) {
        
        if (mode == stepping) get_character();
        if (p.n_dimensional_display) clear_screen();
        
        memcpy(read_cells, write_cells, sizeof read_cells);
        
        for (nat cell = 0; cell < cell_count; cell++) {
            
            if (!(cell % sidelength) && p.n_dimensional_display) puts("");
                        
            fill_balanced_neighbors(n, read_cells, cell, neighborhood, cell_count, sidelength);
            write_cells[cell] = hgrid[unreduce(neighborhood, m, n)];

            const nat slice = cell / sidelength;
            if (timestep >= begin_timestep &&
                ((begin_slice <= slice && slice < end_slice) || (!end_slice && !begin_slice))
                ) {
                if (p.display_as == intuitive_display) graph(read_cells[cell], m);
                else if (p.display_as == numeric_display) printf(" %llu", read_cells[cell]);
                else if (p.display_as == binary_display) fputs(read_cells[cell] ? "##" : "  ", stdout);
            }
        }
        if (timestep >= begin_timestep) {
            puts("");
            if (p.delay) fflush(stdout);
            usleep((unsigned) p.delay);
        }
    }
}

void generate_lifetime_image(const char* filename, nat begin_timestep, nat begin_slice, nat end_slice, vector hgrid, struct parameters p) {
            
    const nat m = p.m, n = p.n, cell_count = p.s, timesteps = p.t, sidelength = p.l;
    
    FILE* file = fopen(filename, "wb");
    
    if (!file) {
        fprintf(stderr, "error: %s: could not open file for writing: %s\n", filename, strerror(errno));
        return;
    }
    
    fprintf(file, "P6\n%llu %llu\n255\n", cell_count, timesteps);
    
    mode = running;
    element neighborhood[n], write_cells[cell_count], read_cells[cell_count];
    initialize(write_cells, m, n, cell_count, sidelength, p.initial_state);
    
    for (nat timestep = 0; timestep < begin_timestep + timesteps && mode != stopped; timestep++) {
        memcpy(read_cells, write_cells, sizeof read_cells);
        for (nat cell = 0; cell < cell_count; cell++) {
            
            fill_balanced_neighbors(n, read_cells, cell, neighborhood, cell_count, sidelength);
            write_cells[cell] = hgrid[unreduce(neighborhood, m, n)];
            
            const nat slice = cell / sidelength;
            if (timestep >= begin_timestep &&
                ((begin_slice <= slice && slice < end_slice)
                 || (!end_slice && !begin_slice))
                ) {
                double x = (double) read_cells[cell] / (double) m;
                unsigned char r = x * 255, g = x * 255, b = x * 255;
                fwrite(&r, 1, 1, file);
                fwrite(&g, 1, 1, file);
                fwrite(&b, 1, 1, file);
            }
        }
    }
    fclose(file);
}

void generate_lifetime_images(char** input, struct context* context) {
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    const char* destination_dir = input[5];
    nat z_count = 0, begin = atoll(input[3]), end = atoll(input[4]);
    vector z_values = read_nats_from_file(input[2], &z_count);
    
    printf("generate: generating %llu lifetimes .ppm's...\n", z_count);
    
    element hgrid[H];
    
    mode = running;
    
    for (nat i = 0; i < z_count && mode != stopped; i++) {
        nat z = z_values[i];
        printf("\r [  %llu  /  %llu  ] : %llu                  "
               , i, z_count, z);
        fflush(stdout);
        
        char path[4096] = {0}, filename[4096] = {0};
        sprintf(filename, "z_%llu.ppm", z);
        strcpy(path, destination_dir);
        strcat(path, "/");
        strcat(path, filename);
        
        reduce(hgrid, z, m, H);
        generate_lifetime_image(path, 0, begin, end, hgrid, context->parameters);
    }
    printf("generate: generated all images.\n");
}

void save_values(const char* out_filename, vector values, nat count) {
    while (true) {
        printf("would you like to write the %llu values to the file \"%s\"? (yes/no) ",
               count, out_filename);
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

void threshold_search(nat threshold, const char* outfile, struct context* context) {
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    vector hgrid_copy = duplicate(context->hgrid, H);
    
    element indicies[H], search[H];
    
    nat indicies_count = find_unknown_indicies(hgrid_copy, H, indicies), search_space = powl(m, indicies_count), z_count = 0;
    element definitions[search_space], scores[search_space];
    
    print_vector_line_message("hgrid", hgrid_copy, H);
    print_vector_line_message("indicies", indicies, indicies_count);
    
    printf("searching over %llu unknowns...\n", indicies_count);
    get_character();
    
    mode = running;
    for (nat z_try = 0; z_try < search_space && mode != stopped; z_try++) {
        
        if (mode == stepping) get_character();
        
        printf("\r [  %llu  /  %llu  ]       ", z_try, search_space);
        fflush(stdout);
        
        reduce(search, z_try, m, indicies_count);
        map(hgrid_copy, search, indicies, H);
        nat score = measure_lifetime(hgrid_copy, &context->parameters), z_definition = unreduce(hgrid_copy, m, H);
        if (score >= threshold) {
            printf("\n[z = %llu] ---> %llu timesteps\n\n", z_definition, score);
            definitions[z_count] = z_definition;
            scores[z_count++] = score;
        }
    }
    puts("\n");
    printf("found %llu z values >= threshold. (%f%%) \n", z_count, 100.0 * ((float)z_count / search_space));
    save_values(outfile, definitions, z_count);
    
    destroy(&hgrid_copy);
}

void target_search(nat target, nat tolerance, const char* outfile, struct context* context) {
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    vector hgrid_copy = duplicate(context->hgrid, H);
    
    element indicies[H], search[H];
    
    nat indicies_count = find_unknown_indicies(hgrid_copy, H, indicies), search_space = powl(m, indicies_count), z_count = 0;
    element definitions[search_space], scores[search_space];
    
    print_vector_line_message("hgrid", hgrid_copy, H);
    print_vector_line_message("indicies", indicies, indicies_count);
    
    printf("searching over %llu unknowns...\n", indicies_count);
    get_character();
    
    mode = running;
    for (nat z_try = 0; z_try < search_space && mode != stopped; z_try++) {
        
        if (mode == stepping) get_character();
        
        printf("\r [  %llu  /  %llu  ]       ", z_try, search_space);
        fflush(stdout);

        reduce(search, z_try, m, indicies_count);
        map(hgrid_copy, search, indicies, H);
        nat score = measure_lifetime(hgrid_copy, &context->parameters), z_definition = unreduce(hgrid_copy, m, H);
        if (score < target + tolerance && score > target - tolerance) {
            printf("\n[z = %llu] ---> %llu timesteps\n\n", z_definition, score);
            definitions[z_count] = z_definition;
            scores[z_count++] = score;
        }
    }
    puts("\n");
    printf("found %llu z values >= threshold. (%f%%) \n", z_count, 100.0 * ((float)z_count / search_space));
    save_values(outfile, definitions, z_count);
    
    destroy(&hgrid_copy);
}


void visualize_set
(nat begin,
 
 nat user_begin_slice,
 nat user_end_slice,
 
 vector set, nat count,
 
 const char* savelist_out_filename,
 const char* blacklist_out_filename,
 
 struct context* context) {
    
    bool should_display = true;
    
    nat offset = 0, at = begin, begin_slice = user_begin_slice, end_slice = user_end_slice;
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    element blacklist[count], savelist[count], hgrid[H];
    
    fill(0, blacklist, count);
    fill(0, savelist, count);
    
    while (true) {
        clear_screen();
        
        if (at < count) {
            nat z = set[at];
            
            if (should_display) {
                reduce(hgrid, z, m, H);
                nat save_delay = context->parameters.delay; context->parameters.delay = 0;
                visualize_lifetime(offset, begin_slice, end_slice, hgrid, context->parameters);
                context->parameters.delay = save_delay;
            }
            
            printf("\n  [z = %llu]  %llu  /  %llu  ", z, at, count);
            if (blacklist[at]) printf("(blacklisted) ");
            if (savelist[at]) printf("(saved) ");
            printf(":> ");
        } else printf("\n  [END]  %llu  /  %llu  :> ", at, count);
        
        fflush(stdout);
        const int c = get_character();
        
        if (c == 'd') offset += context->parameters.t;
        else if (c == 's' && offset >= context->parameters.t) offset -= context->parameters.t;
        
        else if (c == 'f' && at < count) { at++; offset = 0; }
        else if (c == 'e' && at) { at--; offset = 0; }
        
        else if (c == 'w') savelist[at] = !savelist[at];
        else if (c == 'a') blacklist[at] = !blacklist[at];
        else if (c == 't') should_display = !should_display;
        
        else if (c == 'p') {
            printf(" offset: %llu, begin: %llu, end: %llu \n", offset, begin_slice, end_slice);
            fflush(stdout); sleep(1);
        }
        
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
