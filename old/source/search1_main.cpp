//
//  main.cpp
//  search1
//
//  Created by Daniel Rehman on 1909043.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

/*

#include <iostream>
#include <math.h>
#include <string.h>

using nat = size_t; 

int main(int argc, const char * argv[]) {

    if (argc < 5) {
        printf("usage: ./program m n s t\n");
        exit(1);
    }
    
    nat m = atoi(argv[1]), n = atoi(argv[2]), s = atoi(argv[3]), t = atoi(argv[4]);
    nat l = pow(s, n);
    nat g[l], h[l];
    
    memset(g, 0, sizeof g);
    g[0] = 1;       
    
    
    for (nat i = 0; i < t; i++) {
        for (nat j = 0; j < l; j++) {
            g[j] = ;
        }
    }
    
    
    
}




*/

///////////////////////////////








//
//  main.c
//  fast-asl
//
//  Created by Daniel Rehman on 1908051.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include <termios.h>
#include <math.h>



std::string path = "/Users/deniylreimn/Documents/m2n2_orth_ndim_search_of_4096cas.txt";


typedef size_t  nat;
typedef nat*    nats;
using natv = std::vector<nat>; 

bool n_dimensional_display = false;
bool numeric_print = false;

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"


// printers:

static inline void print(const char* message, nats v, nat l) {
    printf("%s = [ ", message);
    for (nat i = 0; i < l; i++) printf("%lu ", v[i]);    
    printf("]\n");
}


static inline void clear_screen() {printf("\e[1;1H\e[2J");}

static inline void display(nat x, nat m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    nat color = (nat)(24.0 * ratio);                // 24 possible valid hues    
    color += 232;                                   // offset to get only the 24 graytone colors.  
    printf(bold_color "██" reset, color);
}

static inline void print(nat* u, nat size, nat L, nat m, nat n, nat timestep) {
    if (n_dimensional_display) clear_screen();    
    else printf("%5lu:   ", timestep);        
    for (nat i = 0; i < L; i++) {        
        if (numeric_print) printf("%4lu ", u[i]);
        else display(u[i], m);        
        if (n_dimensional_display) {
            for (nat q = 1; q < n; q++) {
                nat d = pow(size,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
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

static inline void print_raw(natv v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
}

static inline void print(natv v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}


static inline nat unreduce(nats v, nat radix, nat length) {
    nat s = 0, p = 1;
    for (nat i = 0; i < length; i++, p *= radix) 
        s += p * v[i];
    return s;
}

static inline void reduce(nats out, nat s, nat radix, nat length) {    
    for (nat i = 0, p = 1; i < length; i++, p *= radix) 
        out[i] = (s / p) % radix;
}

//static inline bool not_in(natv v, nat l, nat e) {
//    for (nat i = l; i--;) 
//        if (v[i] == e) return false;
//    return true;
//}

static inline bool not_in(nats v, nat l, nat e) {
    for (nat i = l; i--;) 
        if (v[i] == e) return false;
    return true;
}

static inline void initialize(nats g, nat l) {
    memset(g, 0, sizeof(nat) * l);
    if (l % 2 == 0) {
        for (nat i = 0; i < l / 2; i++) {
            g[i] = 0;
            g[i + 1] = 1;
        }        
    } else g[0] = 1;
}

static inline void print_pert(nats pert, nat pert_length, nat m, nat nc) { 
    std::cout << "printing H grid: \n";
    for (nat i = 0; i < pert_length; i++) {
        printf("%4lu ", pert[i]);
        for (nat q = 1; q < nc; q++) {
            nat d = pow(m, q);
            if (i % d == d - 1) {
                if (nc == 2 or q == 2) printf("\n");
                if (nc == 3 and q == 1) printf("   ");
            }
        }        
    }
}

static inline void initialize_H_grid_with_rule110(nats H, nat length) { // assuming m = 3, n = 1.
    memset(H, 0, sizeof(nat) * length);
}


static inline void populate(nats b) { // assuming 2, 2.

    b[0] = 0;
    b[1] = 1;
    b[2] = 1; //
    b[3] = 1;
    
   
    b[4] = 0; 
    b[5] = 1;
    b[6] = 1; //
    b[7] = 0; //
    
    
    b[8] = 0;
    b[9] = 1;
    b[10] = 1; //
    b[11] = 1;
    
    
    b[12] = 0;
    b[13] = 1;
    b[14] = 1; //
    b[15] = 0; //
        
    
    
    b[16] = 0; //
    b[17] = 1;
    b[18] = 1;
    b[19] = 1;
    
    b[20] = 0; //
    b[21] = 1;
    b[22] = 1;
    b[23] = 0; //
    
    b[24] = 0; //
    b[25] = 1; //
    b[26] = 1;
    b[27] = 1; //
    
    b[28] = 0; //
    b[29] = 1; // 
    b[30] = 1;
    b[31] = 0;
}


struct score_entry {
    double score;
    nat z;
};


struct parameters {
    
    double threshold = 0;
    
    nat m = 0;
    nat n = 0; 
    nat size = 0;    
    nat lifetime = 0;
    nat delay = 0;
    nat mode = 0;        
    
    nat nc = 0;
    nat l = 0;
    nat L = 0;
    nat H = 0;
    nat big_H = 0;
    nat Z = 0;
};

static void write_scores_to_disk(nat score_count, struct score_entry *scores) {
    std::ofstream file {path, std::ios_base::trunc};        
    printf("writing z values to disk...\n");
    for (nat i = 0; i < score_count; i++) {
        auto idx = scores[i].z;
        file << idx << " " << scores[i].score << "\n";
        printf("\r\twriting [ z = %lu  /  %lu ] : %lu ",  i, score_count, idx);
    }
}

static void print_results(double biggest, nat biggest_index, nat score_count, struct score_entry *scores, const parameters &u) {
    printf("\n\n\n------------------------------\n\n");
    printf("there were %lu automatas with a sufficent length.\n", score_count);
    
    for (nat i = 0; i < score_count; i++) {
        auto idx = scores[i].z;
        auto score = scores[i].score;
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lf complexity\n", i, idx, score);
        
        nat Hg[u.H];
        reduce(Hg, idx, u.m, u.H);
        print("\treduced:", Hg, u.H);
        print_pert(Hg, u.H, u.m, u.nc);
    }
    printf("the biggest index found in the search: ");
    printf(" ( %lu ) --> %lf timesteps\n", biggest_index, biggest);
}

bool k_satisfied(nats Hg, nat H) {
    nat zeros_count = 0;
    nat ones_count = 0;
    for (auto i = 0; i < H; i++) {
        if (Hg[i] == 0) zeros_count++;
        else if (Hg[i] == 1) ones_count++;
    }
    assert(zeros_count + ones_count == 32); // assuming 2,2
    return 
        (zeros_count == 15 and ones_count == 17) or
        (ones_count == 15 and zeros_count == 17);
}

static void search(const parameters u) {
    
    nat g[u.L], h[u.L], ns[u.nc], H_grid[u.H];
                
    score_entry* scores = nullptr;
    nat score_count = 0;    
        
    double biggest = 0;
    nat biggest_index = 0;
    
    printf("searching the computational universe...\n");
    
    for (int bit = 1; bit < 31; bit++) { // the flip bit.
        for (int bit1 = 1; bit1 < 31; bit1++) { // the flip bit.
        
            if (bit == bit1) continue;
            
            populate(H_grid);
            
            H_grid[bit] = !H_grid[bit];
            H_grid[bit1] = !H_grid[bit1];
            
            nat z = unreduce(H_grid, u.m, u.H);
            
            initialize(g, u.L);
            nats states = nullptr;
            nat state_count = 0;        
            
            /// find lifetime.        
            while (true) {
                memcpy(h, g, sizeof h);
                for (nat j = u.L; j--;) {
                    ns[0] = h[j];
                    nat y = 1;
                    for (int f = 1; f < u.L; f *= u.size) {
                        ns[y++] = h[j + f * ((j / f + 1) % u.size - j / f % u.size)];
                        ns[y++] = h[j + f * ((j / f + u.size - 1) % u.size - j / f % u.size)];
                    }
                    g[j] = H_grid[unreduce(ns, u.m, u.nc)];
                }
                const nat r = unreduce(h, u.m, u.L);
                if (not_in(states, state_count, r)) {
                    states = (nats) realloc(states, sizeof(nat) * (state_count + 1));
                    states[state_count++] = r;
                }
                else break;
            }
            
            const nat lifetime = state_count;
            const double score = (double)lifetime;                
                
            if (score >= u.threshold) { 
                scores = (struct score_entry*) realloc(scores, sizeof(struct score_entry) * (score_count + 1));
                scores[score_count++] = {score, z};
                printf("FOUND (%lu) -----> %lf \n", z, score);
            }
            if (score > biggest) {
                biggest = score;
                biggest_index = z;
                printf("BIGGEST: ( %lu ) -> %lf\n", biggest_index, biggest);
            }
            write_scores_to_disk(score_count, scores);
            print_results(biggest, biggest_index, score_count, scores, u);
        }
    }            
    
}

static inline void print_usage() {
    printf("usage: ./program m n s mode \n\n"           
           "\t- m : M (nat0) : the modulus, also the size of the CA. \n"
           "\t- n : N (nat0) : the number of dimensions in the CA.\n"               
           "\t- s : N-Cube Side-length (nat0) : How many cells you want to simulate the CA with. L = s^n\n"
           "\t- mode : Mode (bool) : determine how to use this program.\n\n"
           
           "if mode == 0: ./program m n s mode t\n"           
           "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n\n"
           
           "if mode == 1: ./program m n s mode l d nd np \n"           
           "\t- l : Lifetime (nat0) : the number of timesteps to simulate CAs for.\n"           
           "\t- d : delay (nat0) : the delay for usleep. \n" 
           "\t- nd : N-dimensional display (bool) : vizualize the CA N-dimensionally.\n"          
           "\t- np : Numeric print (bool) : print the modnat values as numbers instead of hues.\n"
           
           "\n");
    exit(1);
}

static inline void visualize(const parameters& u) {}


void debug_parameters(parameters u) {
    std::cout << "(m = " << u.m << ", n = " << u.n << "):\n";
    
    std::cout << "\t size = " << u.size << "\n";
    std::cout << "\t mode = " << u.mode << "\n";
    std::cout << "\t threshold = " << u.threshold << "\n";
    std::cout << "\t lifetime = " << u.lifetime << "\n";
    std::cout << "\t delay = " << u.delay << "\n";
    
    std::cout << "computed: \n";
    
    std::cout << "\t m^n = l = " << u.l << "\n";
    std::cout << "\t s^n = L = " << u.L << "\n";
    std::cout << "\t 2n + 1 = nc = " << u.nc << "\n";
    std::cout << "\t m^nc = H = " << u.H << "\n";
    std::cout << "\t m^L = big H = " << u.big_H << "\n";
    std::cout << "\t m^H = Z = " << u.Z << "\n\n";
    
    std::cin.get();
}

parameters compute_paraemters(const char** argv, const int argc) {
        
    parameters u;
    if (argc < 5) print_usage();                
    
    // general parameters:
    u.m = atoi(argv[1]);
    u.n = atoi(argv[2]);
    u.size = atoi(argv[3]);
    u.mode = atoi(argv[4]);
    
    if (u.mode == 1 and argc < 10) print_usage();                
    else if (u.mode == 0 and argc < 6) print_usage();    
    
    if (u.mode == 0) {                      // search parameters:
        u.threshold = atof(argv[5]);        
        
    } else if (u.mode == 1) {               // viz parameters:        
        u.lifetime = atoi(argv[6]);        
        u.delay = atoi(argv[7]);
        n_dimensional_display = atoi(argv[8]); 
        numeric_print = atoi(argv[9]);
    }
    
    // compute seoncdary parameters:
    u.nc = 2 * u.n + 1;
    u.l = pow(u.m, u.n);
    u.L = pow(u.size, u.n);    
    u.H = pow(u.m, u.nc);
    u.big_H = pow(u.m, u.L);
    u.Z = pow(u.m, u.H);
    
    debug_parameters(u);
    return u;
}

int main(int argc, const char * argv[]) {    
    const parameters u = compute_paraemters(argv, argc);    
    if (u.mode == 0) search(u);        
    else if (u.mode == 1) visualize(u);
    else print_usage();
}

