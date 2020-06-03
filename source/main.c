//
//  main.c
//  universe.
//
//  Created by Daniel Rehman on 1910141.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

/*

void print_z_case(element* h_grid, nat lifetime, const struct parameters* u, nat z) {
    printf("[%llu]:  ", z);
    print_vector(h_grid, u->H);
    printf("  --->  %llu timesteps\n", lifetime);
}

void print_results(vector z, nat z_count, vector lifetimes, nat lifetime_count, const struct parameters* u) {
    printf("------------------------- RESULTS --------------------\n\n");
    printf("\tfound %llu automatons above the threshold: \n\n", z_count);
    element h_grid[u->H];
    for (nat i = 0; i < z_count; i++) {
        reduce(h_grid, z[i], u->m, u->H);
        printf("case #%llu  ::  ", i);
        print_z_case(h_grid, lifetimes[i], u, z[i]);
    }
}


void print(vector h, nat count, nat timestep, const struct parameters* u) {
    if (u->n_dimensional_display) clear_screen();
    else printf("%5llu:   ", timestep);
    for (nat i = 0; i < u->L; i++) {
        if (u->numeric_display) printf("%4llu ", h[i]);
        else printf("\033[38;5;%llum" "██" "\033[0m", (nat)(24.0 * (double) h[i] / (double) u->m) + 232);
        if (u->n_dimensional_display) {
            for (nat q = 1; q < u->n; q++) {
                nat d = powl(u->size, q);
                if (i % d == d - 1) {
                    if (u->n == 2 || q == 2) printf("\n");
                    if (u->n == 3 && q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");
    fflush(stdout);
}

void print_h_grid(vector h_grid, const struct parameters* u) {
    element ns[u->nc];
    printf("printing h_grid: \n");
    for (nat h = 0; h < u->H; h++) {
        reduce(ns, h, u->m, u->nc);
        print_vector(ns, u->nc);
        printf(" ---> %llu\n", h_grid[h]);
    }
    printf("\n");
}

void print_expression(vector expression, nat length, const struct parameters* u) { // valid for n <= 3
    printf("{ ");
    for (nat i = 0; i < length; i++) {
        if (expression[i] < u->m) printf("%llu ", expression[i]);
        else if (expression[i] >= u->m && expression[i] < u->nc + u->m) printf("%c ", "CRLUDFB"[expression[i] - u->m]);
        else if (expression[i] >= u->nc + u->m && expression[i] < u->nc + 3 + u->m) printf("%c ", "+-*"[expression[i] - u->nc - u->m]);
        else printf("? ");
    }
    printf("}");
}


// ------------------------- utility ---------------------------



void do_reduce() {
    const nat radix = read_nat("give radix: ");
    const nat z = read_nat("give z to unreduce: ");
    const nat count = 0;
    element out[count];
    reduce(out, z, radix, count);
    print_vector_line_message("result", out, count);
}

void do_unreduce() {
    const nat radix = read_nat("give radix: ");
    const nat count = read_nat("give count: ");
    element v[count];
    read_nats("give vector to unreduce: ", v, count);
    const nat z = unreduce(v, radix, count);
    printf("\n\n\tz = %llu\n\n", z);
}

// ------------------------- RPN evaluator -----------------------------

element eval(vector* stack, nat* count, const nat m,
             const nat nc, vector ns) {
    
    const element add = m + nc + 0, sub = m + nc + 1,
                  mul = m + nc + 2;
    
    element e = pop(stack, count);
    if (e == add) push_back(stack, count, (eval(stack, count, m, nc, ns) + eval(stack, count, m, nc, ns)) % m);
    else if (e == sub) push_back(stack, count, (eval(stack, count, m, nc, ns) + m - eval(stack, count, m, nc, ns)) % m);
    else if (e == mul) push_back(stack, count, (eval(stack, count, m, nc, ns) * eval(stack, count, m, nc, ns)) % m);
    else if (e >= m && e < m + nc) return ns[e - m];
    else return e;
    return back(*stack, *count);
}

void determine_h_grid(vector result, nat H,
                      vector expression, nat expression_length,
                      const nat m, const nat nc) {
    
    element neighborhood[nc];
    for (nat h = 0; h < H; h++) {
        reduce(neighborhood, h, m, nc);
        nat count = expression_length;
        vector save = copy(expression, count);
        result[h] = eval(&save, &count, m, nc, neighborhood);
        destroy(&save);
    }
}

void calcuate_rpn_expression(const struct parameters* u) {
    printf("calculating an h grid from a given rpn expression");
    printf("operators: ");
    print_operators(u->m, u->nc);
    
    printf("CODE: \n"
           "\n\telement e = pop(stack, count);"
           "\n\tif (e == add) push_back(stack, count, (eval(stack, count, m, nc, ns) + eval(stack, count, m, nc, ns)) % m);"
           "\n\telse if (e == sub) push_back(stack, count, (eval(stack, count, m, nc, ns) + m - eval(stack, count, m, nc, ns)) % m);"
           "\n\telse if (e == mul) push_back(stack, count, (eval(stack, count, m, nc, ns) * eval(stack, count, m, nc, ns)) % m);"
           "\n\telse if (e == eq) push_back(stack, count, eval(stack, count, m, nc, ns) == eval(stack, count, m, nc, ns));"
           "\n\telse if (e >= m && e < m + nc) return ns[e - m];"
           "\n\telse return e;\n\n"
           );
    
    nat length = read_nat("give length of rpn: ");
    element rpn[length], h_grid[u->H];
    read_nats("give rpn: ", rpn, length);
    print_vector_line_message("rpn", rpn, length);
    determine_h_grid(h_grid, u->H, rpn, length, u->m, u->nc);
    print_h_grid(h_grid, u);
    print_vector_line_message("compact", h_grid, u->H);
    const nat z = unreduce(h_grid, u->m, u->H);
    printf("\n\tz = %llu\n\n", z);
}


// --------------------- ca utilities ---------------------


void initialize(vector g, const nat m, const nat L, enum initial_state initial) {
    if (initial == empty_state) return;
    else if (initial == dot_state) g[0] = 1;
    else if (initial == repeating_state) {
        for (nat i = 0; i < L; i++) {
            g[i] = i % 2 == 0;
        }
    } else if (initial == random_state) {
        for (nat i = 0; i < L; i++) g[i] = rand() % m;
    }
}

void fill_neighbors(vector h, nat j, vector ns, const nat L, const nat size) {
    ns[0] = h[j];
    nat y = 1;
    for (nat f = 1; f < L; f *= size) {
        ns[y++] = h[j + f * ((j / f + 1) % size - j / f % size)];
        ns[y++] = h[j + f * ((j / f + size - 1) % size - j / f % size)];
    }
}

nat measure_lifetime(vector h_grid, const struct parameters* u) {
    
    vector g = create(u->L);
    vector h = create(u->L);
    element neighborhood[u->nc];
    
    nat state_count = 0;
    vector states = empty_vector;
    initialize(g, u->m, u->L, (enum initial_state) u->initial_state);
    
    while (true) {
        memcpy(h, g, u->L * sizeof(element));
        for (nat j = u->L; j--;) {
            fill_neighbors(h, j, neighborhood, u->L, u->size);
            g[j] = h_grid[unreduce(neighborhood, u->m, u->nc)];
        }
        const nat r = unreduce(h, u->m, u->L);
        if (!in(states, state_count, r)) push_back(&states, &state_count, r);
        else break;
    }
    
    destroy(&states);
    destroy(&h);
    destroy(&g); 
    return state_count;
}

void rpn_search(const struct parameters* u) {
    
    vector lifetimes = empty_vector;
    nat lifetime_count = 0;
    
    vector zs = empty_vector;
    nat z_count = 0;
    
    element h_grid [u->H];
    
    for (nat length = 1; length <= u->max_depth; length += 2) {
        
        printf("------------ trying expr depth = %llu ---------------\n", length);
        
        
        
        ///TODO: remove subtraction from this. its irrelevant.
        ///also, disallow the constant of 0 from showing up in equations.
        ///...?
        
        
        const nat radix = u->m + u->nc + 3; // num ops
        const nat E = powl(radix, length);
        element expression[length];
        printf("\n\n");
        for (nat e = 0; e < E; e++) {
            
            reduce(expression, e, radix, length);
            if (e % 1000 == 0) {
                printf("\r [ %llu / %llu ]  :  trying: ", e, E);
                print_expression(expression, length, u);
                printf("                ");
            }
            
            determine_h_grid(h_grid, u->H, expression, length, u->m, u->nc);
//            print_vector_line_message("---> h grid: ", h_grid, u->H);
            const nat lifetime = measure_lifetime(h_grid, u);
            
            if (lifetime >= u->threshold) {
                push_back(&lifetimes, &lifetime_count, lifetime);
                push_back(&zs, &z_count, e);
                printf("found: %llu:  ", e);
                print_vector(h_grid, u->H);
                printf("  --->  %llu timesteps\n", lifetime);
            }
        }
    }
    destroy(&lifetimes);
    destroy(&zs);
}

void map(vector h_grid, vector known, vector known_indicies, vector search, const struct parameters* u) {
    // assuming :     |search| + |known| = H.
    nat j = 0, k = 0;
    for (nat i = 0; i < u->H; i++) h_grid[i] = i == known_indicies[j] ? known[j++] : search[k++];
}

bool m2n2_condition(nat z, vector h_grid, const struct parameters* u, bool* condition, vector neighborhood, vector g, vector h) {
    bool good = true;

    nat state_count = 0;
    vector states = empty_vector;
    initialize(g, u->m, u->L, (enum initial_state) u->initial_state);
        
    const nat max_lifetime_measure = u->lifetime;
    
    nat t = 0;
    while (good && t < max_lifetime_measure) {
        
        memcpy(h, g, u->L * sizeof(element));
        for (nat j = u->L; j--;) {
            fill_neighbors(h, j, neighborhood, u->L, u->size);
            g[j] = h_grid[unreduce(neighborhood, u->m, u->nc)];
        }
        const nat r = unreduce(h, u->m, u->L);
        if (!in(states, state_count, r)) push_back(&states, &state_count, r);
        else break;
        
        /// check 2d-triangle principle.
        if (t < u->size - 1) {
            for (nat f = 1; f < u->L; f *= u->size) {
                if (h[f * (u->size - 1)]) {
//                    printf("z = %llu : failed 2d-triangle principle @ t = %llu\n", z, t);
                    good = false;
                    break;
                }
            }
        }
        
        /// check non-uniformity principle. (1s and 0s ratio > threshold)
        if (t >= u->size) {
            nat sum = 0;
            const nat count = u->L;
            for (nat j = 0; j < count; j++) sum += h[j];
            double average = ((double) sum) / (double) count;
            
            if (average < u->threshold && average > 1.0 - u->threshold) {
//                printf("z = %llu : failed non-uniformity principle with (%f).\n", z, average);
                
                good = false;
                break;
            }
        }
        
        t++;
    }
    //    printf("z = %llu : succeeded. \n", z);
    destroy(&states);
    *condition = good;
    return state_count;
}

void z_search(const struct parameters* u) {
    
    vector lifetimes = empty_vector, z_values = empty_vector;
    nat lifetime_count = 0, z_count = 0;
    element h_grid[u->H];
    
    nat known_count = read_nat("known count: ");
    element known_indicies[known_count], known_values[known_count];
    read_nats("known indicies: ", known_indicies, known_count);
    read_nats("known values: ", known_values, known_count);
    nat search_H = u->H - known_count;
    nat Z = powl(u->m, search_H);
    element search_vector[search_H];
    
    set(99999999, search_vector, search_H);
    map(h_grid, known_values, known_indicies, search_vector, u);
    printf("known values: (99999999 means its unknown and searched over)\n");
    print_h_grid(h_grid, u);
    set(0, search_vector, search_H);
    map(h_grid, known_values, known_indicies, search_vector, u);
    sleep(1);
    
    user_quit = false;
    user_stop = false;
    user_pause = false;
    
    pthread_t thread;
    pthread_create(&thread, NULL, get_input, NULL);
    
    vector g = create(u->L);
    vector h = create(u->L);
    element neighborhood[u->nc];
    
    for (nat zi = 0; zi < Z; zi++) {
        
        if (user_quit) {user_quit = false; break; }
        while (user_pause) sleep(1);
        
        if (zi % 1024 == 0) printf("\r z = %llu / %llu                ", zi, Z);
        
        reduce(search_vector, zi, u->m, search_H);
        map(h_grid, known_values, known_indicies, search_vector, u);
        const nat z = unreduce(h_grid, u->m, u->H);
        
        bool condition = false;
        const nat lifetime = m2n2_condition(z, h_grid, u, &condition, neighborhood, g, h);
        
        if (condition && lifetime > u->size) {
            push_back(&lifetimes, &lifetime_count, lifetime);
            push_back(&z_values, &z_count, z);
            printf("found: ");
            print_z_case(h_grid, lifetime, u, z);
        }
    }
    
    destroy(&g);
    destroy(&h);
    
    print_results(z_values, z_count, lifetimes, lifetime_count, u);
    
    if (read_nat("write z values to file? ")) {
        printf("writing z values to file...\n");
        write_nats_to_file(u->destination, z_values, z_count);
    }
    user_quit = true;
    pthread_join(thread, NULL);
    destroy(&lifetimes);
    destroy(&z_values);
}

void pert_search(const struct parameters* u) {
    
    vector lifetimes = empty_vector, z_values = empty_vector;
    nat lifetime_count = 0, z_count = 0;
    
    element h_grid[u->H];
    read_nats("enter existing base h_grid(H nats)", h_grid, u->H);
    printf("received h grid: \n");
    print_h_grid(h_grid, u);
    
    element save[u->H];
    memcpy(save, h_grid, sizeof save);
    
    const nat candidate_count = u->H;
    element candidates[candidate_count];
    for (int i = 0; i < candidate_count; i++) candidates[i] = i;
    if (u->m == 2) {
        for (nat i = 0; i < candidate_count; i++) {
            
            printf("------------------- try: %llu -----------------\n", i);
            printf("trying sb = (.1=%llu)...\n", candidates[i]);
            
            h_grid[candidates[i]] = (h_grid[candidates[i]] + 1) % u->m;
            
            
            if (vectors_equal(h_grid, u->H, save, u->H)) {
                printf("skipping this null change of the base...\n");
                continue;
            }
            const nat z = unreduce(h_grid, u->m, u->H);
            
            printf("z = %llu :: ", z);
            print_vector_line(h_grid, u->H);
            const nat lifetime = measure_lifetime(h_grid, u);
            if (lifetime >= u->threshold) {
                push_back(&lifetimes, &lifetime_count, lifetime);
                push_back(&z_values, &z_count, z);
                printf("found: ");
                print_z_case(h_grid, lifetime, u, z);
            }
            memcpy(h_grid, save, sizeof save);
        }
    } else if (u->m == 3) {
        for (nat i = 1; i < candidate_count; i++) {
            for (nat j = 1; j < candidate_count; j++) {
                
                if (candidates[i] == candidates[j]) continue;
                
                printf("------------------- try: %llu,%llu -----------------\n", i, j);
                printf("trying sb = (.1=%llu,.2=%llu)...\n", candidates[i], candidates[j]);
                
                h_grid[candidates[i]] = (h_grid[candidates[i]] + 1) % u->m;
                h_grid[candidates[j]] = (h_grid[candidates[j]] + 2) % u->m;
                
                if (vectors_equal(h_grid, u->H, save, u->H)) {
                    printf("skipping this null change of the base...\n");
                    continue;
                }
                
                const nat z = unreduce(h_grid, u->m, u->H);
                
                printf("z = %llu :: ", z);
                print_vector_line(h_grid, u->H);
                const nat lifetime = measure_lifetime(h_grid, u);
                if (lifetime >= u->threshold) {
                    push_back(&lifetimes, &lifetime_count, lifetime);
                    push_back(&z_values, &z_count, z);
                    printf("found: ");
                    print_z_case(h_grid, lifetime, u, z);
                }
                memcpy(h_grid, save, sizeof save);
            }
        }
    }
    
    print_results(z_values, z_count, lifetimes, lifetime_count, u);
    read_nat("type anything: ");
    read_nat("type anything: ");
    if (read_nat("write z values to file? ")) {
        printf("writing z values to file...\n");
        write_nats_to_file(u->destination, z_values, z_count);
    }
    
    destroy(&lifetimes);
    destroy(&z_values);
}


void visualize_set(struct parameters* u) {
        
    vector g = create(u->L);
    vector h = create(u->L);
    element h_grid[u->H], neighborhood[u->nc];
    user_quit = false;
    user_stop = false;
    user_pause = false;
    user_back = false;
    user_marked = false;
    nat z_count = 0;
    vector z_values = read_nats_from_file(u->source, &z_count);
    
    print_vector_line_message("vizzing the set of z values: ", z_values, z_count);
    sleep(1);
    
    vector marked = empty_vector;
    nat marked_count = 0;
    
//    pthread_t thread;
//    pthread_create(&thread, NULL, get_input, NULL);
    
    for (long long zi = 0; (long long)zi < (long long)z_count; zi++) {
        
        const nat z = z_values[zi];
        reduce(h_grid, z, u->m, u->H);
        
        const nat size_save = u->size;
        const nat L_save = u->L;
        u->size = 16;
        u->L = 16;
        
        const nat lifetime = measure_lifetime(h_grid, u);
        u->size = size_save;
        u->L = L_save;
        
        clear_screen();
        printf("----- (%lld / %llu) vizualizing z = %llu ---> %llu timesteps -------- \n", zi, z_count, z, lifetime);
        print_vector_line(h_grid, u->H);
        
        set(0, g, u->L);
        initialize(g, u->m, u->L, (enum initial_state) u->initial_state);
        
        for (nat t = 0; t < u->lifetime; t++) {
            if (user_quit) {user_quit = false; goto done; }
            if (user_stop) { user_stop = false; break; }
            if (user_back && zi > 0) {user_back = false; zi -= 2; break;}
            while (user_pause) usleep(10000);
            
            if (user_marked) {
                printf("marked this Z value.");
                //                usleep(100000);
                push_back(&marked, &marked_count, z);
                user_marked = false;
                break;
            }
            
            memcpy(h, g, u->L * sizeof(element)); 
            for (nat i = 0; i < u->L; i++) {
                fill_neighbors(h, i, neighborhood, u->L, u->size);
                g[i] = h_grid[unreduce(neighborhood, u->m, u->nc)];
            }
//            if (t % 200 == 0) clear_screen();
            print(h, u->L, t, u);
            usleep((unsigned) u->delay);
        }
        printf("\n\n::> ");
        int c = get_character();
        
        if (c == 'q') break;
        else if (c == 'b' && zi > 0) zi -= 2;
        else if (c == 'm') {
            printf("marked this Z value.");
            push_back(&marked, &marked_count, z);
            user_marked = false;
        }
    }
    done:
//    pthread_join(thread, NULL);
    destroy(&z_values);
    destroy(&g);
    destroy(&h);
    
    printf("marked the following z values: \n");
    print_vector_line(marked, marked_count);
}


void visualize_single(vector h_grid, const struct parameters *u) {
    
    clear_screen();
    
    user_quit = false;
    user_stop = false;
    user_pause = false;
    
    pthread_t thread;
    pthread_create(&thread, NULL, get_input, NULL);
    
    vector g = create(u->L);
    vector h = create(u->L);
    
    set(0, g, u->L);
    initialize(g, u->m, u->L, (enum initial_state) u->initial_state);
    
    for (nat t = 0; t < u->lifetime; t++) {
        if (user_quit) { user_quit = false; break; }
        while (user_pause) sleep(1);
        memcpy(h, g, u->L * sizeof(element));
        for (nat i = 0; i < u->L; i++) {
            element neighborhood[u->nc];
            fill_neighbors(h, i, neighborhood, u->L, u->size);
            g[i] = h_grid[unreduce(neighborhood, u->m, u->nc)];
        }
        print(h, u->L, t, u);
        usleep((unsigned) u->delay);
    }
    pthread_join(thread, NULL);
    destroy(&g);
    destroy(&h);
}

void print_z_h_grid(element *h_grid, const struct parameters *u, nat z) {
    printf("read : %llu = ", z);
    print_vector_line(h_grid, u->H);
    sleep(2);
}

void visualize_single_z(const struct parameters* u) {
    const nat z = read_nat("z: ");
    element h_grid[u->H];
    reduce(h_grid, z, u->m, u->H);
    print_z_h_grid(h_grid, u, z);
    visualize_single(h_grid, u);
}

void visualize_single_h_grid(const struct parameters* u) {
    element h_grid[u->H];
    read_nats("h_grid: ", h_grid, u->H);
    print_z_h_grid(h_grid, u, unreduce(h_grid, u->m, u->H));
    visualize_single(h_grid, u);
}

void compute_derived_parameters(struct parameters* u) {
    u->nc = 2 * u->n + 1;
    u->H = powl(u->m, u->nc);
    u->L = powl(u->size, u->n);
}

void set_parameter(struct parameters* u) {
    char name[128] = {0};
    
    printf("menu: \n\t"
           "m \n\t"
           "n \n\t"
           "size(s) \n\t"
           "lifetime(l) \n\t"
           "delay(d) \n\t"
           "max_depth(md) \n\t"
           "source(S) \n\t"
           "destination(D) \n\t"
           "initial(i) \n\t"
           "threshold(t) \n\t \n");
    
    printf("parameter: ");
    
    fgets(name, 127, stdin);
    name[strlen(name) - 1] = '\0';
    
    if (equals(name, "m", "m")) {
        u->m = read_nat("m: ");
        printf(":: m = %llu\n", u->m);
    }
    
    else if (equals(name, "n", "n")) {
        u->n = read_nat("n: ");
        printf(":: n = %llu\n", u->n);
    }
    
    else if (equals(name, "size", "s")) {
        u->size = read_nat("size: ");
        printf(":: size = %llu\n", u->size);
    }
    
    else if (equals(name, "lifetime", "l")) {
        u->lifetime = read_nat("lifetime: ");
        printf(":: lifetime = %llu\n", u->lifetime);
    }
    
    else if (equals(name, "delay", "d")) {
        u->delay = read_nat("delay: ");
        printf(":: delay = %llu\n", u->delay);
    }
    
    else if (equals(name, "n_dimensional_display", "nd")) {
        u->n_dimensional_display = read_nat("n_dimensional_display: ");
        printf(":: nd = %d\n", u->n_dimensional_display);
    }
    
    else if (equals(name, "numeric_display", "np")) {
        u->numeric_display = read_nat("numeric_display: ");
        printf(":: numeric_display = %d\n", u->numeric_display);
    }
    
    else if (equals(name, "max_depth", "md")) {
        u->max_depth = read_nat("max_depth: ");
        printf(":: max_depth = %llu\n", u->max_depth);
    }
    
    else if (equals(name, "threshold", "t")) {
        u->threshold = read_float("threshold: ");
        printf(":: threshold = %f\n", u->threshold);
    }
    
    else if (equals(name, "source", "S")) {
        free(u->source);
        nat max_path_length = 256;
        u->source = malloc(sizeof(char) * max_path_length);
        printf("source: ");
        fgets(u->source, 255, stdin);
        u->source[strlen(u->source) - 1] = 0;
        printf(":: source = %s\n", u->source);
    }

    else if (equals(name, "destination", "D")) {
        free(u->destination);
        nat max_path_length = 256;
        u->destination = malloc(sizeof(char) * max_path_length);
        printf("destination: ");
        fgets(u->destination, 255, stdin);
        u->destination[strlen(u->destination) - 1] = 0;
        printf(":: destination = %s\n", u->destination);
    }
    
    else if (equals(name, "initial", "i")) {
        printf("Initial confifguration (0/1/2/3) : \n\t0 for empty, \n\t1 for dot, \n\t2, for repeating, \n\t3 for random.\n\n");
        u->initial_state = read_nat("initial: ");
        printf(":: initial_state = %llu\n", u->initial_state);
    }
    
    compute_derived_parameters(u);
}

*/

#include "structures.h"
#include "parameters.h"
#include "vector.h"
#include "hgrid.h"
#include "ca.h"
#include "io.h"

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

int main() {
    
//    struct parameters u = {0};
    
    bool quit = false;
    char input[2048] = {0};
    
    while (!quit) {
        printf(" 〉");
        fgets(input, sizeof input, stdin);
        input[strlen(input) - 1] = '\0';
  
        if (equals(input, "quit", "q")) quit = true;
        else if (equals(input, "help", "h")) printf("help is coming!\n");
        else if (equals(input, "", "")) {}
        else printf("unknown command: \"%s\"\n", input);
    }
}
