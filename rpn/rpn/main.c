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
// #include <stdbool.h>      // this isnt working, for some reason.
   
typedef size_t nat;

typedef int_fast32_t bool;
const bool false = 0;
const bool true = 1;

enum initial_state {
    empty_state,
    dot_state,
    repeating_state,
    random_state,
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
    const char* destination;
    /// the file path source used to vizualize sequential automatons.
    const char* source;
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

void zero(vector v, nat count) {
    memset(v, 0, sizeof(element) * count);
}

void push_back(vector* v, nat* count, element e) {
    *v = realloc(*v, sizeof(element) * (*count + 1));
    (*v)[(*count)++] = e;
}

void pop_back(vector* v, nat* count) {
    if (!*count) abort();
    (*count)--;
    *v = realloc(*v, sizeof(element) * (*count));
}

element pop(vector* v, nat* count) {
    if (!*count) abort();
    (*count)--;
    element e = (*v)[*count];
    *v = realloc(*v, sizeof(element) * (*count));
    return e;
}

element back(vector v, nat count) {
    if (!count) abort();
    return v[count - 1];
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
    if (strlen(a) != strlen(b)) return false;
    for (nat i = 0; i < strlen(a); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
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

void print_vector(vector v, nat count) {
    printf("{ ");
    for (nat i = 0; i < count; i++) {
        printf("%d ", (int) v[i]);
    }
    printf("}\n");
}

void read_vector(vector* v, nat *count) {
    char buffer[64] = {0};
    
    printf("give length: ");
    fgets(buffer, 63, stdin);
    int n = atoi(buffer);
    
    for (int i = 0; i < n; i++) {
        printf("[%d]: ", i);
        fgets(buffer, 63, stdin);
        int k = atoi(buffer);
        push_back(v, count, k);
    }
}

nat read_nat(const char* message) {
    char buffer[64] = {0};
    printf("%s", message);
    fgets(buffer, 63, stdin);
    return atoi(buffer);
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



// ------------------------- RPN evaluator -----------------------------

element eval(vector* stack, nat* count, const nat m, const nat nc, vector ns) {
    
    const element
        add = m + nc + 0,
        sub = m + nc + 1,
        mul = m + nc + 2,
        eq  = m + nc + 3;
    
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
    
    vector neighborhood = create(nc);
    for (nat h = 0; h < H; h++) {
        reduce(neighborhood, h, m, nc);
        nat count = expression_length;
        vector save = copy(expression, count);
        result[h] = eval(&save, &count, m, nc, neighborhood);
        destroy(&save);
    }
    destroy(&neighborhood);
}


// --------------------- ca utilities ---------------------


void initialize(vector g, const nat m, const nat L, enum initial_state initial) {
    if (initial == empty_state) return;
    else if (initial == dot_state) g[0] = 1;
    else if (initial == repeating_state) {
        for (nat i = 0; i < L / 2; i++) {
            g[i] = 0;
            g[i + 1] = 1;
        }
    } else if (initial == random_state) {
        for (nat i = 0; i < L; i++) g[i] = rand() % m;
    }
}

void fill_neighbors(vector h, nat j, vector ns, const nat L, const nat size) {
    ns[0] = h[j];
    nat y = 1;
    for (int f = 1; f < L; f *= size) {
        ns[y++] = h[j + f * ((j / f + 1) % size - j / f % size)];
        ns[y++] = h[j + f * ((j / f + size - 1) % size - j / f % size)];
    }
}

nat measure_lifetime(vector h_grid, const struct parameters u) {
    
    vector g = create(u.L);
    vector h = create(u.L);
    element neighborhood[u.nc];
    
    nat state_count = 0;
    vector states = empty_vector;
    
    initialize(g, u.m, u.L, dot_state);
    
    while (true) {
        h = g;
        for (nat j = u.L; j--;) {
            fill_neighbors(h, j, neighborhood, u.L, u.size);
            g[j] = h_grid[unreduce(neighborhood, u.m, u.nc)];
        }
        const nat r = unreduce(h, u.m, u.L);
        if (!in(states, state_count, r)) push_back(&states, &state_count, r);
        else break;
    }
    
    destroy(&states);
    destroy(&h);
    destroy(&g);
    return state_count;
}


void clear_screen() {printf("\e[1;1H\e[2J");}

void print(vector h, nat count, nat timestep, const struct parameters u) {
    if (u.n_dimensional_display) clear_screen();
    else printf("%5lu:   ", timestep);
    for (nat i = 0; i < u.L; i++) {
        if (u.numeric_display) printf("%4lu ", h[i]);
        else printf("\033[38;5;%lum" "██" "\033[0m", (nat)(24.0 * (double) h[i] / (double) u.m) + 232);
        if (u.n_dimensional_display) {
            for (nat q = 1; q < u.n; q++) {
                nat d = pow(u.size, q);
                if (i % d == d - 1) {
                    if (u.n == 2 || q == 2) printf("\n");
                    if (u.n == 3 && q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");
    fflush(stdout);
}


char getch() {
    struct termios t = {0}; if (tcgetattr(0, &t) < 0) perror("tcsetattr()");
    t.c_lflag &= ~ICANON; t.c_lflag &= ~ECHO; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t) < 0) perror("tcsetattr ICANON");
    char c = 0; if (read(0, &c, 1) < 0) perror("read()"); t.c_lflag |= ICANON; t.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &t) < 0) perror("tcsetattr ~ICANON");
    return c;
}


bool visualize_quit = false;
bool visualize_stop = false;
bool visualize_pause = false;

void* visualize_input(void* unused) {
    while (!visualize_quit) {
        char c = getch();
        if (c == 'q') visualize_quit = true;
        if (c == 'c') visualize_stop = true;
        else if (c == 'p') visualize_pause = true;
        else if (c == 'u') visualize_pause = false;
    }
    return unused;
}

void visualize(const struct parameters u) {
        
    vector g = create(u.L);
    vector h = create(u.L);
    vector h_grid = create(u.H);
    element neighborhood[u.nc];
    
//    auto z_values = read_nats(u.source);
    nat z_count = 0;
    vector z_values = create(0);
    
    pthread_t thread;
    pthread_create(&thread, NULL, visualize_input, NULL);
    
    for (nat zi = 0; zi < z_count; zi++) {
        const nat z = z_values[zi];
        
        if (visualize_quit) {visualize_quit = false; break; }
        printf("----- vizualizing z = %lu: -------- \n", z);
        
        reduce(h_grid, z, u.m, u.H);
        print_vector(h_grid, u.H);
        initialize(g, u.m, u.L, u.initial_state);
        
        for (nat t = 0; t < u.lifetime; t++) {
            if (visualize_stop) { visualize_stop = false; break; }
            while (visualize_pause) sleep(1);
            
            h = g;
            for (nat i = 0; i < u.L; i++) {
                fill_neighbors(h, i, neighborhood, u.L, u.size);
                g[i] = h_grid[unreduce(neighborhood, u.m, u.nc)];
            }
            print(h, u.L, t, u);
            usleep((unsigned) u.delay);
        }
        sleep(1);
    }
    pthread_join(thread, NULL);
}


void compute_derived_parameters(struct parameters* u) {
    u->nc = 2 * u->n + 1;
    u->H = pow(u->m, u->nc);
    u->L = pow(u->size, u->n);
}


void set_parameter(struct parameters* u) {
    char name[128] = {0};
    
    printf("parameter: ");
    
    fgets(name, 127, stdin);
    name[strlen(name) - 1] = '\0';
    
    if (strings_equal(name, "m")) {
        u->m = read_nat("m: ");
        printf(":: m = %lu\n", u->m);
    }
    
    else if (strings_equal(name, "n")) {
        u->n = read_nat("n: ");
        printf(":: n = %lu\n", u->n);
    }
    
    else if (strings_equal(name, "size")) {
        u->size = read_nat("size: ");
        printf(":: size = %lu\n", u->size);
    }
    
    else if (strings_equal(name, "lifetime")) {
        u->lifetime = read_nat("lifetime: ");
        printf(":: lifetime = %lu\n", u->lifetime);
    }
    
    else if (strings_equal(name, "delay")) {
        u->delay = read_nat("delay: ");
        printf(":: delay = %lu\n", u->delay);
    }
    
    else if (strings_equal(name, "max_depth")) {
        u->max_depth = read_nat("max_depth: ");
        printf(":: max_depth = %lu\n", u->max_depth);
    }
    
//    else if (strings_equal(name, "source")) {
//        u->size = read_nat("size: ");
//        printf(":: size = %lu\n", u->size);
//    }
//
//    else if (strings_equal(name, "destination")) {
//        u->destination = read_nat("destination: ");
//        printf(":: destination = %s\n", u->destination);
//    }
       
    compute_derived_parameters(u);
}

void print_parameters(struct parameters u) {
    printf("current parameters: \n");
    printf("\tm = %lu\n", u.m);
    printf("\tn = %lu\n", u.n);
    printf("\tsize = %lu\n", u.size);
    printf("\tlifetime = %lu\n", u.lifetime);
    printf("\tdelay = %lu\n", u.delay);
    printf("\tmax_depth = %lu\n", u.max_depth);
    printf("\tnd = %d\n", u.n_dimensional_display);
    printf("\tnp = %d\n", u.numeric_display);
    printf("\tthr = %f\n", u.threshold);
    printf("\tdest = %s\n", u.destination);
    printf("\tsource = %s\n", u.source);
    
    printf("computed parameters:\n");
    printf("\tnc = %lu\n", u.nc);
    printf("\tH = %lu\n", u.H);
    printf("\tL = %lu\n", u.L);
    printf("\n");
}


void print_help() {
    printf("commands:\n"
           "\t- quit: quit the ca terminal.\n"
           "\t- help print this help menu.\n"
           "\t- help parameters: print parametre descriptions.\n"
           "\t- clear: clear the screen.s\n"
           "\t- get: get the current set of paraemtrs.\n"
           "\t- set: set a given paraemter, by name.s\n"
           "\t- visualize: visuzlize a single, or a set of z values.\n"
           "\t- search: search the ca space.\n"
           );
}

void print_parameter_descriptions() {
    printf("parameter semantics: \n"
           "\t- m : M (nat0) : the modulus, also the size of the CA. \n"
           "\t- n : N (nat0) : the number of dimensions in the CA.\n"
           "\t- s : N-Cube Side-length (nat0) : How many cells you want to simulate the CA with. L = s^n\n"
           "\t- i : Initial confifguration (0/1/2/3) : 0 for empty, 1 for dot, 2, for repeating, 3 for random. \n"
           "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n"
           "\t- max_depth (nat0) : the maximum depth of symbolic expressions which will be considered. \n"
           "\t- Destination (file) : all found z values will be written here (truncs the file.) \n\n"
           "\t- l : Lifetime (nat0) : the number of timesteps to simulate CAs for.\n"
           "\t- d : delay (nat0) : the delay for usleep. eg, 100000 \n"
           "\t- nd : N-dimensional display (0/1) : vizualize the CA N-dimensionally.\n"
           "\t- np : Numeric print (0/1) : print the modnat values as numbers instead of hues.\n"
           "\t- Source (file) : file containing all z values which will be vizualized. \n\n"
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
        .size = 3,
        .lifetime = 100,
        .delay = 100000,
        .max_depth = 2,
        .threshold = 0.0,
        .n_dimensional_display = false,
        .numeric_display = false,
        .initial_state = dot_state,
        .destination = "",
        .source = ""
    };
    
    compute_derived_parameters(&u);
    
    print_welcome();
    
    bool quit = false;
    char input[128] = {0};
    while (!quit) {
        
        printf(":> ");
        fgets(input, 127, stdin);
        input[strlen(input) - 1] = '\0';
  
        if (strings_equal(input, "quit")) quit = true;
        else if (strings_equal(input, "help")) print_help();
        else if (strings_equal(input, "help parameters")) print_parameter_descriptions();
        else if (strings_equal(input, "clear")) clear_screen();
        else if (strings_equal(input, "get")) print_parameters(u);
        else if (strings_equal(input, "set")) set_parameter(&u);
        else if (strings_equal(input, "visualize")) visualize(u);
        else if (strings_equal(input, "search")) printf("unimplemented.\n");
        else printf("unknown command: \"%s\"\n", input);
    }
    exit(0);
}
