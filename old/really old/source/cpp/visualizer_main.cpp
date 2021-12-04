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
#include <iostream>
#include <vector>

typedef size_t  nat;
typedef nat*    nats;

void print(const char* message, nats v, nat l) {
    printf("%s = [ ", message);
    for (nat i = 0; i < l; i++) printf("%lu ", v[i]);    
    printf("]\n");
}

bool n_dimensional_display = false;
bool numeric_print = false;

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"

void clear_screen() {printf("\e[1;1H\e[2J");}

void display(nat x, nat m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    nat color = (nat)(24.0 * ratio);          // 24 possible valid hues    
    color += 232;                                   // offset to get only the 24 graytone colors.  
    printf(bold_color "██" reset, color);
}

void print(nat* u, nat l, nat n, nat m, nat i, nat z) {
    if (n_dimensional_display) clear_screen();
    
    if (not n_dimensional_display) printf("%5lu:   ", i);    
    
    for (nat i = 0; i < l; i++) {
        if (numeric_print) printf("%4lu ", u[i]);
        else display(u[i], m);
        if (n_dimensional_display) {
            for (nat q = 1; q < n; q++) {
                nat d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");    
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

static inline bool not_in(std::vector<nat> v, nat l, nat e) {
    for (nat i = l; i--;) 
        if (v[i] == e) return false;
    return true;
}

int main(int argc, const char * argv[]) {
    
    if (argc < 8) {
        printf("usage: ./program m n t thr \n\n"
               "\t- M (nat0) : the modulus, also the size of the CA. \n"
               "\t- N (nat0) : the number of dimensions in the CA.\n"
               "\t- T (nat0) : the number of timesteps to simulate CAs for.\n"
               "\t- Threshold (nat0) : CA lifetimes found to be below this threshold will not be reported. \n" 
               "\t- N-dimensional display (bool) : vizualize the CA N-dimensionally.\n"          
               "\t- Numeric print (bool) : print the modnat values as numbers instead of hues.\n"
               "\n");
        exit(1);
    }
    
    const nat 
    m = atoi(argv[1]), 
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    delay = atoi(argv[4]),
    nc = n + 1,
    l = pow(m, n), 
    H = pow(m, nc);                
    n_dimensional_display = atoi(argv[5]);
    numeric_print = atoi(argv[6]);            
    bool should_break = atoi(argv[7]);
    
    printf("CONSTANTS: \n");
    printf("\tm = %lu\n", m);
    printf("\tn = %lu\n", n);
    printf("\tnc = %lu\n", nc);
    printf("\tl = %lu\n", l);
    printf("\tH = %lu\n", H);
    
    nat g[l], h[l], ns[nc], perturbator[H];  

    for (nat i = 0; i < H; i++) {
        nat h = 0;
        std::cout << "give pert[" << i << "]: ";
        std::cin >> h;
        perturbator[i] = h;
    }
    
    std::vector<nat> states = {};
    
    memset(g, 0, sizeof g);                        
    g[0] = 1;    
    
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
        if (not_in(states, states.size(), r)) states.push_back(r);
        else if (should_break) break;
        
        print(h, l, n, m, i, 0);
        usleep((unsigned) delay);
    }
}
