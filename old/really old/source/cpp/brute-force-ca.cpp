//
//  main.c
//  fast-asl
//
//  Created by Daniel Rehman on 1908051.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

typedef size_t  nat;
typedef nat*    nats;

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
    l = pow(m, n),
    H = pow(m, l),     
    Z = pow(m, H);
        
    nat g[l], h[l], perturbator[H], states[H];
    nats keepers = NULL;
    nat master_count = 0;
    
    nat biggest = 0;
    nat biggest_index = 0;
    
    printf("searching the VSBN computational universe...\n");
    
    for (nat z = 0; z < Z; z++) {
        if (z % 10000 == 0) printf("\r [ z = %lu  /  %lu ]", z, Z);
        
        reduce(perturbator, z, m, H);
        memset(g, 0, sizeof g);
        g[0] = 1;
        nat state_count = 0;
        
        for (nat i = t; i--;) {
            memcpy(h, g, sizeof h);
            const nat r = unreduce(h, m, l);
            for (nat j = l; j--;) {
                g[j] = perturbator[r];
            }
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
    
    printf("there were %lu automatas with a sufficent length.\n", master_count / 2);
    for (nat i = 0; i < master_count; i++) {
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lu timesteps\n", i / 2, keepers[i+1], keepers[i]);
        i++;
    }       
    printf("the biggest index found in the search: \n");
    printf(" ( %lu ) --> %lu timesteps", biggest_index, biggest);
    return 0;
}
