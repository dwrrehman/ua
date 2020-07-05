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

static inline bool not_in(natv v, nat l, nat e) {
    for (nat i = l; i--;) 
        if (v[i] == e) return false;
    return true;
}

static inline bool not_in(nats v, nat l, nat e) {
    for (nat i = l; i--;) 
        if (v[i] == e) return false;
    return true;
}

static inline void initialize(nats g, nat l) {
    memset(g, 0, sizeof(nat) * l); 
    g[0] = 1;
}

static inline void initialize_special(nats g, nat l) {
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


static void search(nat H, nat Z, nat big_H, nat l, nat m, nat nc, nat t, nat n) {
    nat g[l], h[l], ns[nc], H_grid[H], states[big_H];
    
    nats keepers = NULL;
    nat master_count = 0;
    
    nat biggest = 0;
    nat biggest_index = 0;
    
    printf("searching the computational universe...\n");        
    
    for (nat z = 0; z < Z; z++) {
        
        printf("\r\t[ z = %lu  /  %lu ] ", z, Z);
        reduce(H_grid, z, m, H);
        initialize(g, l);
        nat state_count = 0;
        
        while (true) {
            memcpy(h, g, sizeof h);
            for (nat j = l; j--;) {
                ns[0] = h[j];
                nat y = 1;
                for (int f = 1; f < l; f *= m) {
                    ns[y++] = h[j + f * ((j / f + 1) % m - j / f % m)];
                    ns[y++] = h[j + f * ((j / f + m - 1) % m - j / f % m)];
                }
                g[j] = H_grid[unreduce(ns, m, nc)];
            }
            const nat r = unreduce(h, m, l);
            if (not_in(states, state_count, r)) states[state_count++] = r;
            else break;
        }
        if (state_count >= t) {
            keepers = (nats) realloc(keepers, sizeof(nat) * (master_count + 2));
            keepers[master_count++] = state_count;
            keepers[master_count++] = z;
            printf("FOUND (%lu) -----> %lu \n", z, state_count);            
            print("\treduced:", H_grid, H);
            print_pert(H_grid, H, m, nc); 
        }
        if (state_count > biggest) {
            biggest = state_count;
            biggest_index = z;
            printf("BIGGEST: ( %lu ) -> %lu\n", biggest_index, biggest);
            print("\treduced:", H_grid, H);
            print_pert(H_grid, H, m, nc);
        } 
    }
    printf("\n\n\n------------------------------\n\n");
    printf("there were %lu automatas with a sufficent length.\n", master_count / 2);
    for (nat i = 0; i < master_count; i++) {
        auto idx = keepers[i + 1];
        nat Hg[H];
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lu timesteps\n", i / 2, idx, keepers[i]);
        reduce(Hg, idx, m, H);
        print("\treduced:", Hg, H);
        print_pert(Hg, H, m, nc);        
        i++;
    }
    printf("the biggest index found in the search: \n");
    printf(" ( %lu ) --> %lu timesteps", biggest_index, biggest);
}

static inline void print_usage() {
    printf("usage: ./program m n mode \n\n"           
            "\t- m : M (nat0) : the modulus, also the size of the CA. \n"
            "\t- n : N (nat0) : the number of dimensions in the CA.\n"               
            "\t- mode : Mode (bool) : determine how to use this program.\n\n"
           
        "if mode == 0: ./program m n mode t\n"           
            "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n\n"
           
        "if mode == 1: ./program m n mode l s d nd np \n"           
            "\t- l : Lifetime (nat0) : the number of timesteps to simulate CAs for.\n"
            "\t- s : N-Cube Side-length (nat0) : How many cells you want to simulate the CA with.\n"
            "\t- d : delay (nat0) : the delay for usleep. \n" 
            "\t- nd : N-dimensional display (bool) : vizualize the CA N-dimensionally.\n"          
            "\t- np : Numeric print (bool) : print the modnat values as numbers instead of hues.\n"
           
            "\n");
    exit(1);
}

//static inline void get_pert(nat H, nat *perturbator) {
//    std::cout << "give pert: ";
//    for (nat i = 0; i < H; i++) {
//        nat h = 0;        
//        std::cin >> h;
//        perturbator[i] = h;
//    }
//}

static inline void visualize(nat H, nat delay, nat l, nat lifetime, 
                             nat m, nat n, nat nc, nat size) {
    
    const nat L = pow(size, n);
    nat g[L], h[L], ns[nc], perturbator[H];
    
    nat z = 0;
    std::cout << "give z: ";
    std::cin >> z;
    
    reduce(perturbator, z, m, H);      
    
    initialize_special(g, L);
    
    for (nat i = 0; i < lifetime; i++) {
        memcpy(h, g, sizeof h);
        for (nat j = L; j--;) {
            ns[0] = h[j];
            nat y = 1;
            for (int f = 1; f < L; f *= size) {
                ns[y++] = h[j + f * ((j / f + size + 1) % size - j / f % size)];
                ns[y++] = h[j + f * ((j / f + size - 1) % size - j / f % size)];
            }
            g[j] = perturbator[unreduce(ns, m, nc)]; 
        }
        
        print(h, size, L, m, n, i);
        usleep((unsigned) delay);
    }
}

int main(int argc, const char * argv[]) {
    if (argc < 4) print_usage();
    const nat
    m = atoi(argv[1]),
    n = atoi(argv[2]),
    mode = atoi(argv[3]),    
    nc = 2 * n + 1,
    l = pow(m, n),
    H = pow(m, nc),
    big_H = pow(m, l),       
    Z = pow(m, H);    
    
    if (mode == 0) {
        if (argc < 5) print_usage();
        const nat t = atoi(argv[4]);
        search(H, Z, big_H, l, m, nc, t, n);
        
    } else if (mode == 1) {
        if (argc < 9) print_usage();                
        const nat 
        lifetime = atoi(argv[4]),
        size = atoi(argv[5]),
        delay = atoi(argv[6]);
        n_dimensional_display = atoi(argv[7]); 
        numeric_print = atoi(argv[8]);                
        visualize(H, delay, l, lifetime, m, n, nc, size);  
    } else {
        std::cout << "Error: invalid mode.\n";
        print_usage();
    }
}

