//
//  main.cpp
//  sbr-finder
//
//  Created by Daniel Rehman on 1907254.
//                                                       
//

#include <iostream>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <unistd.h>

bool n_dimensional_display = true;
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
    for (int i = 0; i < argc; i++) {
        printf("\t# %d: \"%s\"\n", i, argv[i]);
    }
    printf("]\n");
}



//// vector functions:

std::vector<size_t> vector(size_t* g, size_t m, size_t n, size_t l) {
    std::vector<size_t> result {};
    result.resize(l, 0);
    for (int i = 0; i < l; i++) {
        result[i] = g[i];
    }
    return result;
} 

bool are_equal(std::vector<size_t> a, std::vector<size_t> b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
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




std::vector<size_t> reduce(size_t s, size_t m, size_t l) {
    std::vector<size_t> result = {};
    for (size_t p = 1; p < l; p *= m) {
        result.push_back((s/p) % m);
    }
    return result;
}



size_t special_case(size_t* g, size_t* h,       // returns either 1 or 0. 
                    size_t m, size_t n, 
                    size_t j, size_t s, 
                    /*size_t w,*/ size_t l) {
    if (are_equal(reduce(s, m, l), vector(h, m, n, l))) return 1;  
    //if (are_equal(reduce(w, m, l), vector(h, m, n, l))) return 1; 
    else return 0;
}


int main(int argc, const char * argv[]) {
    
    print_args(argv, argc);
    
    if (argc < 7) {
        printf("usage: ./ads M-nat0 N-nat0 delay-nat Nd-bool NP-bool timesteps-nat\n\n");
        exit(1);
    }    
    
    size_t 
    m = 2, //atoi(argv[1]),
    n = 1, //atoi(argv[2]),
    l = pow(m, n), g[l], h[l];
    int delay = atoi(argv[3]);
    n_dimensional_display = atoi(argv[4]);
    numeric_print = atoi(argv[5]);
    size_t t = atoi(argv[6]);
    memset(g, 0, sizeof g);
    g[0] = 1;
    
    
    size_t s_count = pow(m, l);         // number of possible neighborhood configurations.    
    size_t lengths[s_count];
    memset(lengths, 0, sizeof(size_t) * s_count);
    
    for (size_t s = 0; s < s_count; s++) {
        
        lengths[s] = 0;
        
        std::cout << "trying SBR: " << s << "\n";                
        sleep(1);
        
        std::vector<std::vector<size_t>> states = {};
        
        memset(g, 0, sizeof g);
        g[0] = 1;
        
        for (size_t i = 0; i < t; i++) {
            memcpy(h, g, sizeof h);
            for (size_t j = l; j--;) {
                for (size_t k = l; k--;)
                    for (size_t f = 1; f < l; f *= m)
                        g[j] = (g[j] + h[k] * ((j/f + k/f)) * ((j/f + k/f))) % m;
                g[j] = (g[j] + special_case(g, h, m, n, j, s, l)) % m;
            }
            
            if (not contains(states, vector(h, m, n, l))) {
                lengths[s]++;
                states.push_back(vector(h, m, n, l));
            }
            
            //if (n_dimensional_display) clear_screen();
            print(h, l, n, m, i);
            //usleep(delay); 
        }
                        
        std::cout << "for (s) = (" << s << "), length = " << lengths[s] << "\n";
    }
    return 0;
}
