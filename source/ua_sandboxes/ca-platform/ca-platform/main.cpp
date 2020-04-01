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
//    size_t i = 1;
//    for (auto index : perturbator) {
//        if (index == h_index) {
//            return i;s
//        }
//        i++;
//    }    
    return 0;
}


size_t double_odd() {
    
    return 0;
}
size_t double_even() {
    return 0;
}

size_t single_odd() {
    return 0;
}
size_t singe_even() {
    return 0;
}






void initialize(size_t* g, size_t l) {
    memset(g, 0, sizeof(size_t) * l);
    g[0] = 1;
}

int main(int argc, const char * argv[]) {
    
    print_args(argv, argc);    
    
    if (argc < 7) {
        printf("usage: ./ads " // 0
               "M-nat0 " // 1
               "N-nat0 " // 2
               "t-nat0 " // 3
               "delay-nat0 " // 4 
               "Nd-bool " // 5
               "NP-bool \n\n"); // 6
        exit(1);
    }    
    
    size_t
    m = atoi(argv[1]),
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    l = pow(m, n);
    
    unsigned delay = atoi(argv[4]);
    n_dimensional_display = atoi(argv[5]);
    numeric_print = atoi(argv[6]);
    
    
    std::vector<size_t> pertubator = {};
    pertubator.resize(m - 1, 0);
    
    std::cout << "give m - 1 z values: "; 
    for (int i = 0; i < m - 1; i++) {
        std::cin >> pertubator[i];
    }
    
    std::cout << "simulating CA with z_set: ";
    print(pertubator);
    std::cout << "\n\n";
    sleep(1);
                    
    size_t g[l], h[l];        
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
        if (n_dimensional_display) clear_screen();
        print(h, l, n, m, i);
        usleep(delay);
    }
}
