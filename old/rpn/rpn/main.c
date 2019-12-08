//
//  main.c
//  rpn
//
//  Created by Daniel Rehman on 1910141.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

typedef unsigned long long int nat;

enum initial_state {
    empty_state,
    dot_state,
    repeating_state,
    random_state
};

struct parameters {
    /// the modulus.
    nat m;
    /// the dimension.
    nat n;
    /// the side length of the ncube.
    nat size;
    /// the lifetime duration for visualization.
    nat lifetime;
    /// the delay in microseconds for visualization.
    nat delay;
    /// the maximum expression depth for symbolic search.
    nat max_depth;
    /// the search score threshold to save to a file.
    double threshold;
    /// should visualize the CA n dimensionally, as opposed to serialize over time.
    bool n_dimensional_display;
    /// display the CA as a set of numbers.
    bool numeric_display;
    /// the initial configuration of the space.
    nat initial_state;
    /// the file path destintation for the scores and z values found using search method.
    char* destination;
    /// the file path source used to vizualize sequential automatons.
    char* source;
    // ------ computed parameters: ------
    /// nc = 2n + 1 = "neighborhood count"
    nat nc;
    /// L = size ^ n = "total cells"
    nat L;
    /// H = m ^ nc = "neighborhood state count"
    nat H;
};


typedef nat element;
typedef element* vector;
element* const empty_vector = NULL;

vector rule110 = (element[]) {0, 1, 1, 1, 0, 1, 1, 0};

bool user_quit = false;
bool user_stop = false;
bool user_pause = false;


// ----------------- vector functions --------------------

vector create(nat count) {
    return calloc(count, sizeof(element));
}

void destroy(vector* v) {
    free(*v);
    *v = empty_vector;
}

vector copy(vector v, nat count) {
    vector c = create(count);
    memcpy(c, v, sizeof(element) * count);
    return c;
}

void set(nat value, vector v, nat count) {
    memset(v, value, sizeof(element) * count);
}

void push_back(vector* v, nat* count, element e) {
    *v = realloc(*v, sizeof(element) * (*count + 1));
    (*v)[(*count)++] = e;
}

void pop_back(vector* v, nat* count) {
    if (!*count) return;
    (*count)--;
    *v = realloc(*v, sizeof(element) * (*count));
}

element pop(vector* v, nat* count) {
    if (!*count) return 0; // temp
    (*count)--;
    element e = (*v)[*count];
    *v = realloc(*v, sizeof(element) * (*count));
    return e;
}

element back(vector v, nat count) {
    if (!count) return 0; // temp
    return v[count - 1];
}

nat sum(vector v, nat count) {
    nat s = 0;
    for (nat i = 0; i < count; i++) s += v[i];
    return s;
}

// ------------------------ utilities -------------------------


bool vectors_equal(vector a, nat a_count, vector b, nat b_count) {
    if (a_count != b_count) return false;
    for (nat i = 0; i < a_count; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}


bool strings_equal(const char* a, const char* b) {
    return !strcmp(a, b);
}


bool equals(const char* a, const char* b, const char* abbrev) {
    return strings_equal(a, b) || strings_equal(a, abbrev);
}

nat unreduce(vector v, nat radix, nat length) {
    nat s = 0, p = 1;
    for (nat i = 0; i < length; i++, p *= radix)
        s += p * v[i];
    return s;
}

void reduce(vector out, nat s, nat radix, nat length) {
    for (nat i = 0, p = 1; i < length; i++, p *= radix)
        out[i] = (s / p) % radix;
}

bool in(vector v, nat count, element e) {
    for (nat i = 0; i < count; i++) {
        if (v[i] == e) return true;
    }
    return false;
}


// ----------------- IO functions --------------------


void clear_screen() {printf("\e[1;1H\e[2J");}

char getch() {
    struct termios t = {0}; if (tcgetattr(0, &t) < 0) perror("tcsetattr()");
    t.c_lflag &= ~ICANON; t.c_lflag &= ~ECHO; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t) < 0) perror("tcsetattr ICANON");
    char c = 0; if (read(0, &c, 1) < 0) perror("read()"); t.c_lflag |= ICANON; t.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &t) < 0) perror("tcsetattr ~ICANON");
    return c;
}

void* get_input(void* unused) {
    while (!user_quit) {
        char c = getch();
        if (c == 'q') user_quit = true;
        if (c == 'c') user_stop = true;
        else if (c == 'p') user_pause = true;
        else if (c == 'u') user_pause = false;
    }
    return unused;
}

void print_vector(vector v, nat count) {
    printf("{ ");
    for (nat i = 0; i < count; i++) printf("%llu ", v[i]);
    printf("}");
}

void print_vector_line(vector v, nat count) {
    printf("{ ");
    for (nat i = 0; i < count; i++) printf("%llu ", v[i]);
    printf("}\n");
}

nat read_nat(const char* message) {
    char buffer[128] = {0};
    printf("%s", message);
    fgets(buffer, 127, stdin);
    return atoll(buffer);
}

float read_float(const char* message) {
    char buffer[128] = {0};
    printf("%s", message);
    fgets(buffer, 127, stdin);
    return atof(buffer);
}

void read_nats(const char* message, vector v, nat count) {
    printf("%s\n", message);
    for (nat i = 0; i < count; i++) {
        printf("[%llu]: ", i);
        v[i] = read_nat("");
    }
}

vector read_nats_from_file(const char* filepath, nat* count) {
    vector v = empty_vector;
    FILE* file = fopen(filepath, "r");
    if (!file) { perror("fopen"); return empty_vector; }
    char buffer[256] = {0};
    while(fgets(buffer, 255, file)) push_back(&v, count, atoll(buffer));
    fclose(file);
    return v;
}

void write_nats_to_file(const char* filepath, vector v, nat count) {
    FILE* file = fopen(filepath, "w+");
    if (!file) { perror("fopen"); return; }
    for (nat i = 0; i < count; i++) fprintf(file, "%llu\n", v[i]);
    fclose(file);
}

void print_operators(nat m, nat nc) {
    const element
        add = m + nc + 0,
        sub = m + nc + 1,
        mul = m + nc + 2,
        eq  = m + nc + 3;
    
    printf("+: %d\n", (int) add);
    printf("-: %d\n", (int) sub);
    printf("*: %d\n", (int) mul);
    printf("=: %d\n", (int) eq);
}


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


// ------------------------- RPN evaluator -----------------------------

element eval(vector* stack, nat* count, const nat m,
             const nat nc, vector ns) {
    
    const element add = m + nc + 0, sub = m + nc + 1,
                  mul = m + nc + 2, eq  = m + nc + 3;
    
    element e = pop(stack, count);
    if (e == add) push_back(stack, count, (eval(stack, count, m, nc, ns) + eval(stack, count, m, nc, ns)) % m);
    else if (e == sub) push_back(stack, count, (eval(stack, count, m, nc, ns) + m - eval(stack, count, m, nc, ns)) % m);
    else if (e == mul) push_back(stack, count, (eval(stack, count, m, nc, ns) * eval(stack, count, m, nc, ns)) % m);
    else if (e == eq) push_back(stack, count, eval(stack, count, m, nc, ns) == eval(stack, count, m, nc, ns));
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
    initialize(g, u->m, u->L, u->initial_state);
    
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
    
    for (nat length = 1; length < u->max_depth; length += 2) {
        
        printf("trying expr depth = %llu\n", length);
        
        const nat radix = u->m + u->nc + 4;
        const nat Z = powl(radix, length);
        
        element expression[length];
        
        for (nat z = 0; z < Z; z++) {
            
            reduce(expression, z, radix, length);
            determine_h_grid(h_grid, u->H, expression, length, u->m, u->nc);
            const nat lifetime = measure_lifetime(h_grid, u);
            
            if (lifetime >= u->threshold) {
                push_back(&lifetimes, &lifetime_count, lifetime);
                push_back(&zs, z_count, z);
                printf("found: %llu:  ", z);
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
    
    for (nat zi = 0; zi < Z; zi++) {
        
        if (user_quit) {user_quit = false; break; }
        while (user_pause) sleep(1);
        
        if (zi % 1024 == 0) printf("\r z = %llu / %llu                ", zi, Z);
        
        reduce(search_vector, zi, u->m, search_H);
        map(h_grid, known_values, known_indicies, search_vector, u);
        const nat z = unreduce(h_grid, u->m, u->H);
        const nat lifetime = measure_lifetime(h_grid, u);
        
        if (lifetime >= u->threshold) {
            push_back(&lifetimes, &lifetime_count, lifetime);
            push_back(&z_values, &z_count, z);
            printf("found: ");
            print_z_case(h_grid, lifetime, u, z);
        }
    }
    
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



void sb_search(const struct parameters* u) {
    
    vector lifetimes = empty_vector, z_values = empty_vector;
    nat lifetime_count = 0, z_count = 0;
    
    
    element h_grid[u->H];
    printf("enter existing base h_grid(H nats): ");
    for (int i = 0; i < u->H; i++) {
        scanf("%llu", &h_grid[i]);
    }
    printf("received h grid: \n");
    print_h_grid(h_grid, u);
 
    
    const nat candidate_count = read_nat("give the number of candidates (<H): ");
    element candidate_indicies[candidate_count];
    printf("enter candidate indicies: ");
    for (int i = 0; i < u->H; i++) {
        scanf("%llu", &candidate_indicies[i]);
    }
    printf("received candidate indicies in h grid.");

    for (nat i = 0; i < candidate_count; i++) {
        for (nat m = 0; m < u->m; m++) {
            printf("------------------- try: %llu -----------------\n", i);
            printf("trying sb = %llu...\n", candidate_indicies[i]);
            h_grid[candidate_indicies[i]] = m;
            const nat z = unreduce(h_grid, u->m, u->H);
            printf("z = %llu\n", z);
            print_h_grid(h_grid, u);
            const nat lifetime = measure_lifetime(h_grid, u);
            if (lifetime >= u->threshold) {
                push_back(&lifetimes, &lifetime_count, lifetime);
                push_back(&z_values, &z_count, z);
                printf("found: ");
                print_z_case(h_grid, lifetime, u, z);
            }
        }
    }
    
    print_results(z_values, z_count, lifetimes, lifetime_count, u);
    
    if (read_nat("write z values to file? ")) {
        printf("writing z values to file...\n");
        write_nats_to_file(u->destination, z_values, z_count);
    }
    
    destroy(&lifetimes);
    destroy(&z_values);
}










void visualize_set(const struct parameters* u) {
        
    vector g = create(u->L);
    vector h = create(u->L);
    element h_grid[u->H], neighborhood[u->nc];
    user_quit = false;
    user_stop = false;
    user_pause = false;
    nat z_count = 0;
    vector z_values = read_nats_from_file(u->source, &z_count);
    
    pthread_t thread;
    pthread_create(&thread, NULL, get_input, NULL);
    
    for (nat zi = 0; zi < z_count; zi++) {
        const nat z = z_values[zi];
        reduce(h_grid, z, u->m, u->H);
        
        clear_screen();
        printf("----- vizualizing z = %llu: -------- \n", z);
        print_vector_line(h_grid, u->H);
        
        set(0, g, u->L);
        initialize(g, u->m, u->L, u->initial_state);
        
        for (nat t = 0; t < u->lifetime; t++) {
            if (user_quit) {user_quit = false; goto done; }
            if (user_stop) { user_stop = false; break; }
            while (user_pause) sleep(1);
            
            memcpy(h, g, u->L * sizeof(element)); 
            for (nat i = 0; i < u->L; i++) {
                fill_neighbors(h, i, neighborhood, u->L, u->size);
                g[i] = h_grid[unreduce(neighborhood, u->m, u->nc)];
            }
            print(h, u->L, t, u);
            usleep((unsigned) u->delay);
        }
        sleep(1);
    }
    done:
    pthread_join(thread, NULL);
    destroy(&z_values);
    destroy(&g);
    destroy(&h);
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
    initialize(g, u->m, u->L, u->initial_state);
    
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

void print_parameters(struct parameters* u) {
    printf("current parameters: \n");
    printf("\tm = %llu\n", u->m);
    printf("\tn = %llu\n", u->n);
    printf("\tsize = %llu\n", u->size);
    printf("\tlifetime = %llu\n", u->lifetime);
    printf("\tdelay = %llu\n", u->delay);
    printf("\tmax_depth = %llu\n", u->max_depth);
    printf("\tnd = %d\n", u->n_dimensional_display);
    printf("\tnp = %d\n", u->numeric_display);
    printf("\tthr = %f\n", u->threshold);
    printf("\tdest = %s\n", u->destination);
    printf("\tsource = %s\n", u->source);
    printf("\tinitial_state = %llu\n", u->initial_state);
    
    printf("computed parameters:\n");
    printf("\tnc = %llu\n", u->nc);
    printf("\tH = %llu\n", u->H);
    printf("\tL = %llu\n", u->L);
    printf("\n");
}

void print_help() {
    printf("commands:\n"
           "\t- quit(q): quit the ca terminal.\n"
           "\t- help(h): print this help menu.\n"
           "\t- help parameters(hp): print parametre descriptions.\n"
           "\t- clear(l): clear the screen.\n"
           "\t- get(.): get the current set of paraemtrs.\n"
           "\t- set(,): set a given paraemter, by name.\n"
           "\t- visualize(v): visuzlize a set of z values, from source file\n"
           "\t- visualize z(z): visualizes a single z value.\n"
           "\t- visualize h grid(hg): visualizes a single z value, by h grid.\n"
           "\t- search(s): search the ca space, using z values as the substrate. writes to the dest.\n"
           "\t- symbolic search(ss): search the ca space, using rpn expressions as the substrate.\n"
           );
}

void print_parameter_descriptions() {
    printf("parameter semantics: \n"
           "\t- m : M (nat0) : the modulus, also the size of the CA. \n"
           "\t- n : N (nat0) : the number of dimensions in the CA.\n"
           "\t- s : N-Cube Side-length (nat0) : How many cells you want to simulate the CA with. L = s^n\n"
           "\t- i : Initial confifguration (0/1/2/3) : 0 for empty, 1 for dot, 2, for repeating, 3 for random. \n"
           "\t- t : Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n"
           "\t- md : max_depth (nat0) : the maximum depth of symbolic expressions which will be considered. \n"
           "\t- D : Destination (file) : all found z values will be written here (truncs the file.) \n\n"
           "\t- l : Lifetime (nat0) : the number of timesteps to simulate CAs for.\n"
           "\t- d : delay (nat0) : the delay for usleep. eg, 100000 \n"
           "\t- nd : N-dimensional display (0/1) : vizualize the CA N-dimensionally.\n"
           "\t- np : Numeric print (0/1) : print the modnat values as numbers instead of hues.\n"
           "\t- S : Source (file) : file containing all z values which will be vizualized. \n\n"
           "\t- D : Destination (file) : file which will contain all z values which were found from searching. \n\n"
           "commands for the visualizer: \n"
           "\t- q to stop the viz of all automatons \n"
           "\t- c to continue to the next z value \n"
           "\t- p/u to pause and unpause the simulation.\n"
           "\n");
}

void print_welcome() {
    printf("welcome to the cellular automaton terminal.\n"
           "type \"help\" for a list of commands.\n");
}


int main(int argc, const char * argv[]) {
    
    struct parameters u = {
        .m = 3,
        .n = 1,
        .size = 10,
        .lifetime = 100,
        .delay = 10000,
        .max_depth = 0,
        .threshold = 0.0,
        .n_dimensional_display = false,
        .numeric_display = false,
        .initial_state = dot_state,
        .destination = NULL,
        .source = NULL
    };
    
    compute_derived_parameters(&u);
    
    print_welcome();
    
    bool quit = false;
    char input[128] = {0};
    while (!quit) {
        
        printf(":: ");
        fgets(input, 127, stdin);
        input[strlen(input) - 1] = '\0';
  
        if (equals(input, "quit", "q")) quit = true;
        else if (equals(input, "help", "h")) print_help();
        else if (equals(input, "help parameters", "hp")) print_parameter_descriptions();
        else if (equals(input, "clear", "l")) clear_screen();
        else if (equals(input, "get", ".")) print_parameters(&u);
        else if (equals(input, "set", ",")) set_parameter(&u);
        else if (equals(input, "visualize", "v")) visualize_set(&u);
        else if (equals(input, "visualize z", "z")) visualize_single_z(&u);
        else if (equals(input, "visualize h grid", "hg")) visualize_single_h_grid(&u);
        else if (equals(input, "symolic search", "ss")) rpn_search(&u);
        else if (equals(input, "search", "s")) z_search(&u);
        else if (equals(input, "sb search", "sb")) sb_search(&u);
        else if (equals(input, "", "")) {}
        else printf("unknown command: \"%s\"\n", input);
    }
    free(u.source);
    free(u.destination);
    exit(0);
}
