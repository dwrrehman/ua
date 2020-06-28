//
//  main.cpp
//  bf-m3
//
//  Created by Daniel Rehman on 1907287.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <vector>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <unistd.h>

bool n_dimensional_display = false;
bool numeric_print = false;

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"

void clear_screen() {printf("\e[1;1H\e[2J");}

void display(size_t x, size_t m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    size_t color = (size_t)(24.0 * ratio);          // 24 possible valid hues    
    color += 232;                                   // offset to get only the 24 graytone colors.  
    printf(bold_color "██" reset, color);        
    
}

void print(std::vector<size_t> v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}

void print(size_t* u, size_t l, size_t n, size_t m, size_t i) {
    if (not n_dimensional_display) printf("%5lu:   ", i);
    for (size_t i = 0; i < l; i++) {
        if (numeric_print) printf("%4lu ", u[i]);
        else display(u[i], m);
        if (n_dimensional_display) {
            for (size_t q = 1; q < n; q++) {
                size_t d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");
}

void print_args(const char** argv, int argc) {
    printf("[");
    for (size_t i = 0; i < argc; i++) {
        printf("\t# %lu: \"%s\"\n", i, argv[i]);
    }
    printf("]\n");
}


//// vector functions:

std::vector<size_t> vector(size_t* g, size_t l) {
    std::vector<size_t> result {};
    result.resize(l, 0);
    for (size_t i = 0; i < l; i++) {
        result[i] = g[i];
    }
    return result;
} 

bool are_equal(std::vector<size_t> a, std::vector<size_t> b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool contains(std::vector<std::vector<size_t>> container, std::vector<size_t> tofind) {
    for (auto element : container) {
        if (are_equal(element, tofind)) return true;
    }
    return false;
}

size_t unreduce(size_t* h, size_t l, size_t m) {
    size_t result = 0; 
    size_t p = 1;
    for (size_t i = l; i--;) {
        result += p * h[i];
        p *= m;
    }
    return result;
}


std::vector<size_t> reduce(size_t s, size_t m, size_t n, size_t l) {
    std::vector<size_t> result = {};
    result.reserve(n);
    for (size_t p = 1; p < l; p *= m) {
        result.push_back((s/p) % m);
    }
    return result;
}



size_t get_pert(std::vector<size_t> perturbator, size_t h_index) { // pert has size m - 1.
    size_t i = 1;
    for (auto index : perturbator) {
        if (index == h_index) {
            return i;
        }
        i++;
    }    
    return 0;
}



void initialize(size_t* g, size_t l) {
    memset(g, 0, sizeof(size_t) * l);
    g[0] = 1;
}

int main(int argc, const char * argv[]) {
    
    
    print_args(argv, argc);    
    
    if (argc < 5) {
        printf("usage: ./ads " // 0
               "M-nat0 " // 1
               "N-nat0 " // 2
               "t-nat0 " // 3
               "Thr-nat0 \n\n"); // 4
        exit(1);
    }    
        
    size_t
    m = atoi(argv[1]),
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    threshold = atoi(argv[4]),
    l = pow(m, n),
    H = pow(m, l),
    Z = pow(H, m - 1);
    
    std::vector<std::pair<size_t, size_t>> lengths {};
    
    std::cout << "searching computational universe...\n\n";
    sleep(1);
        
    for (size_t z = 0; z < Z; z++) {
        
        printf("\r[ z = %lu  /  %lu ] ", z, Z);

        auto pertubator = reduce(z, H, m - 1, Z);

        size_t g[l], h[l];
        std::vector<std::vector<size_t>> states = {};
        initialize(g, l);
        
        for (size_t i = t; i--;) {
            memcpy(h, g, sizeof h);
            for (size_t j = l; j--;) {
                g[j] = 0;
                for (size_t k = l; k--;) {
                    for (size_t f = 1; f < l; f *= m) {
                        g[j] = (g[j] + h[k] * ((j/f + k/f)) * ((j/f + k/f))) % m;
                    }
                }
                g[j] = (g[j] + get_pert(pertubator, unreduce(h, l, m))) % m;
            }
            auto v = vector(h, l);
            if (not contains(states, v)) states.push_back(v);
        }
        if (states.size() > threshold) lengths.push_back({z, states.size()});
    }
    
    std::cout << "finding longest lifetime...\n";
    sleep(1);
    
    short biggest = 0;
    std::vector<size_t> indices = {};
    
    for (auto pair : lengths) {
        if (pair.second > biggest) {
            biggest = pair.second;
            indices = {pair.first};
        } else if (pair.second == biggest) {
            indices.push_back(pair.first);    
        }
    }
    
    std::cout << "longest lifetimes: \n\tz = "; print(indices);
    std::cout << "\tlasted " << biggest << " timesteps.\n";
    
    std::cout << "\n reduced indicies: ";    
    for (auto i : indices) {
        print(reduce(i, H, m - 1, Z));
    }
}


/*

lengths[s][w] = 0;

std::cout << "trying (s, w) = (" << s << ", " << w << ")\n"; 
//sleep(1);

std::vector<std::vector<size_t>> states = {};

memset(g, 0, sizeof g);
g[0] = 1;
for (size_t i = 0; i < t; i++) {
    memcpy(h, g, sizeof h);
    for (size_t j = l; j--;) {
        for (size_t k = l; k--;)
            for (size_t f = 1; f < l; f *= m)
                g[j] = (g[j] + h[k] * ((j/f + k/f)) * ((j/f + k/f))) % m;
        g[j] = (g[j] + special_case(g, h, m, n, j, s, w, l)) % m;
    }
    
    if (not contains(states, vector(h, m, n, l))) {
        lengths[s][w]++;
        states.push_back(vector(h, m, n, l));
    }
    
    //if (n_dimensional_display) clear_screen();
    print(h, l, n, m, i);
    //usleep(delay);
}

//sleep(1);
std::cout << "for (s, w) = (" << s << ", " << w << "), length = " << lengths[s][w] << "\n";
//sleep(2);
}
*/











/*
size_t M = 100;
size_t L = pow(M, 10);

for (size_t i = 0; i < L; i++) {
    std::cout << "i = " << i << "\n";
    
    std::cout << "r(i) = [ ";
    for (auto e : reduce(i, M, L)) {
        std::cout << e << " ";
    } std::cout << "]\n";
    
    std::cout << "u(r(i)) = " << unreduce(reduce(i, M, L), M);
    
    }
    
    exit(1);
    
*/
