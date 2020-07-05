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
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#include <iostream>
#include <vector>

typedef size_t  nat;
typedef nat*    nats;
using natv = std::vector<nat>; 

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

static inline void print(const char* message, nats v, nat l) {
    printf("%s = [ ", message);
    for (nat i = 0; i < l; i++) printf("%lu ", v[i]);    
    printf("]\n");
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

static inline bool not_in(nats v, nat l, nat e) {
    for (nat i = l; i--;) 
        if (v[i] == e) return false;
    return true;
}

static inline void initialize(nats g, nat l) {
    memset(g, 0, sizeof(nat) * l); 
    g[0] = 1;
}

static inline nat factorial(nat n) {    
    nat r = 1;
    for (nat i = n; i--;) {
        r *= i + 1;
    }
    return r;
}


void init_hg(nats H_grid, nat m, nat l) {
    nat e = 0;
    for (nat i = 0; i < m; i++) {
        for (nat j = l; j--;) {
            H_grid[e++] = (i + 1) % m;
        }
    }    
}


static inline void generate(nats H_grid, nat z, nat l, nat m, nat flip_rows)  {
    
    init_hg(H_grid, m, l);
    
    
    nat flips[m * flip_rows];
    memset(flips, 0, sizeof flips);    
    reduce(flips, z, 2, m * flip_rows);
    
    //print("flips", flips, m * flip_rows);
    
    nat e = 0;
    
    for (nat j = 0; j < flip_rows; j++) {
        for (nat i = 0; i < m; i++) {        
            if (flips[e++]) {
                H_grid[j * 2 * m + i] = (H_grid[j * 2 * m + i] + 1 + m) % m;
                H_grid[(j * 2 + 1) * m + i] = (H_grid[(j * 2 + 1) * m + i] - 1 + m) % m;
            }
        }
    }
}



static inline void print_pert(nats pert, nat m) {
    std::cout << "printing n = 1 perturabtor H grid: \n";
    for (nat i = 0; i < m; i++) {
        std::cout << "\t";
        for (nat j = 0; j < m; j++) {
            std::cout << pert[j + i * m] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


int main(int argc, const char * argv[]) {
    
    if (argc < 4) {
        printf("usage: ./program m n t \n\n"
               "\t- M (nat0) : the modulus, also the size of the CA. \n"
               "\t- N (nat0) : the number of dimensions in the CA.\n"               
               "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n\n");
        exit(1);
    }
    
    const nat
    m = atoi(argv[1]),
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    
    nc = 2 * n + 1,
    l = pow(m, n),
    Hnc = pow(m, nc),
    big_H = pow(m, l),       
    Z = pow(m, Hnc);    

    nat g[l], h[l], ns[nc], H_grid[Hnc], states[big_H];
    
    nats keepers = NULL;
    nat master_count = 0;
    
    nat biggest = 0;
    nat biggest_index = 0;
    
    printf("searching the computational universe...\n");        
    
    for (nat z = 0; z < Z; z++) {
        
        reduce(H_grid, z, m, Hnc);
        
        printf("\r\t[ z = %lu  /  %lu ] ", z, Z); 
                                
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
            print("\treduced:", H_grid, Hnc);
            print_pert(H_grid, m);
        }
        if (state_count > biggest) {
            biggest = state_count;
            biggest_index = z;
            printf("BIGGEST: ( %lu ) -> %lu\n", biggest_index, biggest);
            print("\treduced:", H_grid, Hnc);
            print_pert(H_grid, m);
        } 
    }
    printf("\n\n\n------------------------------\n\n");
    printf("there were %lu automatas with a sufficent length.\n", master_count / 2);
    for (nat i = 0; i < master_count; i++) {
        auto idx = keepers[i + 1];
        nat Hg[Hnc];  
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lu timesteps\n", i / 2, idx, keepers[i]);
        reduce(Hg, idx, m, Hnc);
        print("\treduced:", Hg, Hnc);
        print_pert(Hg, m);        
        i++;
    }
    printf("the biggest index found in the search: \n");
    printf(" ( %lu ) --> %lu timesteps", biggest_index, biggest);
    return 0;
}
