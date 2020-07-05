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

void print_raw(natv v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
}


void print(natv v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}

void print_args(const char** argv, int argc) {
    printf("[");
    for (nat i = 0; i < argc; i++) {
        printf("\t# %lu: \"%s\"\n", i, argv[i]);
    }
    printf("]\n");
}

bool contains(natv container, nat a) {
    for (auto e : container) if (e == a) return true; return false;
}


natv vector(nat* g, nat l) {
    natv result {};
    result.resize(l, 0);
    for (nat i = 0; i < l; i++) {
        result[i] = g[i];
    }
    return result;
}  


void print(const char* message, nats v, nat l) {
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





static inline void print_pert(natv pert, nat m) {
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





static inline natv 
construct_H_grid(
                 nats K,  // the serizlized grid of CRUX values. assuming the (6,1) case. if so, this is a 3 x 3 grid of modnats in the bottom right corner of the H grid vizualization. there are about a million possible configurations of the CRUZ for (6,1).
                 nat S,  // the slice column index: the slice is a column of values which are incremented by 1 according to the norm.
                 nat I, // the initial value of the normal cases.  
                 nat H,  // cardinaity of the pertubtator (the result of this function)
                 nat m // the modulus.
                 )
{ // returns the resultant pertubator based on these parameters for the construction of the H grid.
    
    nat pert[m][m]; // assuming nc = n + 1 = 2, because n = 1.  
    
    /// initialize the canonical form, using the initial starting value, "I" : 
    
    for (nat i = 0; i < m; i++) {
        for (nat j = 0; j < m; j++) {
            pert[i][j] = (I + m - j) % m;
        }
    }
    
    /// install the slice, at the slice column index, "S":
    
    for (int i = 0; i < m; i++) {
        pert[S][i]++;
    } 
    
    
    /// install the CRUX values:
    
    nat p = 0;
    
    for (nat i = 3; i < m; i++) {
        for (nat j = 3; j < m; j++) {
            pert[j][i] = K[p++];
        }
    }

//    
//    /// assuming just the minimal crux: l
//    
//    pert[3][3] = K[0];
//    pert[4][3] = K[1];
//    pert[3][4] = K[2];
//    
//    
    
    /// construct result:
    
    natv result = {};    
    for (nat i = 0; i < m; i++) {
        for (nat j = 0; j < m; j++) {
            result.push_back(pert[j][i]);
        }
    }
    
    //print(result);
    return result;    
}






int main(int argc, const char * argv[]) {
    
    if (argc < 5) {
        printf("usage: ./program m n t thr \n\n"
               "\t- M (nat0) : the modulus, also the size of the CA. \n"
               "\t- N (nat0) : the number of dimensions in the CA.\n"
               "\t- T (nat0) : the number of timesteps to simulate CAs for.\n"
               "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n\n");
        exit(1);
    }
    
    const nat 
    m = atoi(argv[1]), 
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    thr = atoi(argv[4]), 
    nc = n + 1,  
    l = pow(m, n),
    pert_size = pow(m, nc), 
    Q = 9,                                     // outdated: // 9 M's for k,            2 for s and i.     but we will manually choose S and I.
    big_H = pow(m, l),
    Z = pow(m, Q); // m^9.
    
    printf("CONSTANTS: \n");
    printf("\tm = %lu\n", m);
    printf("\tn = %lu\n", n);
    printf("\tnc = %lu\n", nc);
    printf("\tl = %lu\n", l);    
    printf("\tbig H = %lu\n", big_H);
    printf("\tZ = %lu\n\n", Z); 
    
    nat g[l], h[l], ns[nc], states[big_H];
    
    natv perturbator(pert_size, 0);
    
    nats keepers = NULL;
    nat master_count = 0;
    
    nat biggest = 0;
    nat biggest_index = 0;

    const nat S = m / 2;
    const nat I = m / 2;
    
    printf("searching the VSBN computational universe...\n");
    
    for (nat z = 0; z < Z; z++) {
        
        if (z % 256 == 0) printf("\r [ k_z = %lu  /  %lu ]", z, Z);
    
        nat K[Q];
        reduce(K, z, m, Q);
        perturbator = construct_H_grid(K, S, I, pert_size, m);

        memset(g, 0, sizeof g);
        g[0] = 1;
        nat state_count = 0;
        
        for (nat i = t; i--;) {
            memcpy(h, g, sizeof h);
            for (nat j = l; j--;) {
                g[j] = 0;
                for (nat k = l; k--;) 
                    for (nat f = 1; f < l; f *= m) 
                        g[j] = (g[j] + h[k] * ((j/f + k/f)) * ((j/f + k/f))) % m;
                                
                ns[0] = h[j];
                nat y = 1;
                for (int f = 1; f < l; f *= m)
                    ns[y++] = h[j + f * ((j / f + 1) % m - j / f % m)];
                
                g[j] = (g[j] + perturbator[unreduce(ns, m, nc)]) % m;
            }
            const nat r = unreduce(h, m, l);            
            if (not_in(states, state_count, r)) states[state_count++] = r;
            else break;
        }
        if (state_count >= thr) {
            keepers = (nats) realloc(keepers, sizeof(nat) * (master_count + 2));
            keepers[master_count++] = state_count;
            keepers[master_count++] = z;
            printf("FOUND (%lu) -----> %lu \n", z, state_count);
        }
        if (state_count > biggest) {
            biggest = state_count;
            biggest_index = z;
            printf("BIGGEST: ( %lu ) -> %lu\n", biggest_index, biggest);
        } 
    }
    
    printf("there were %lu automatas with a sufficent length.\n", master_count);
    for (nat i = 0; i < master_count; i++) {
        auto idx = keepers[i+1];
        nat pert[pert_size];
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lu timesteps\n", i / 2, idx, keepers[i]);
        reduce(pert, idx, m, pert_size);
        print("\tpert", pert, pert_size);
        i++;
    }       
    printf("the biggest index found in the search: \n");
    printf(" ( %lu ) --> %lu timesteps", biggest_index, biggest);
    return 0;
}
