///
//  main.cpp
//  bf-m3
//
//  Created by Daniel Rehman on 1907287.
//

#include <iostream>
#include <vector>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <unistd.h>

using nat = std::size_t;
using nats = std::vector<nat>;

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

void print_raw(nats v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
}


void print(nats v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}

void print(nat* u, nat l, nat n, nat m, nat i, nat z) {
    if (n_dimensional_display) clear_screen();
    
    if (not n_dimensional_display) printf("%5lu:   ", i);
    else std::cout << "z = " << z << ": \n";
    
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

void print_args(const char** argv, int argc) {
    printf("[");
    for (nat i = 0; i < argc; i++) {
        printf("\t# %lu: \"%s\"\n", i, argv[i]);
    }
    printf("]\n");
}



//// vector functions:

nats vector(nat* g, nat l) {
    nats result {};
    result.resize(l, 0);
    for (nat i = 0; i < l; i++) {
        result[i] = g[i];
    }
    return result;
} 

bool are_equal(nats a, nats b) {
    if (a.size() != b.size()) return false;
    for (nat i = 0; i < a.size(); i++) 
        if (a[i] != b[i]) return false;    
    return true;
}

bool contains(std::vector<nats> container, nats tofind) {
    for (auto element : container) 
        if (are_equal(element, tofind)) return true;    
    return false;
}

bool contains(nats container, nat a) {
    for (auto e : container) if (e == a) return true; return false;
}

nat unreduce(nats vector, nat radix, nat vector_length) {
    nat result = 0;
    nat power = 1;
    for (nat index = 0; index < vector_length; index++) {
        result += power * vector[index];
        power *= radix;
    }
    return result;
}

nats reduce(nat to_reduce, nat radix, nat vector_length) {
    nats vector = {}; 
    vector.reserve(vector_length);    
    for (nat power = 1; power < pow(radix, vector_length); power *= radix)        
        vector.push_back((to_reduce / power) % radix);    
    return vector;
}


void print_vsbn(nats ns, nat m, nat n, nat l, nat j) {        // ns is the indicies of neighbors, and has length = DE or SO or etc...
    for (nat i = 0; i < l; i++) {
        
        if (i == j and contains(ns, i)) {
            std::cout << "[Q]";
        } else if (i == j) {
            std::cout << "[O]";            
        } else if (contains(ns, i)) { 
            std::cout << "[X]";            
        } else {
            std::cout << "[ ]";
        }
        
        for (nat q = 1; q < n; q++) {
            nat d = pow(m,q);
            if (i % d == d - 1) {
                if (n == 2 or q == 2) printf("\n");
                if (n == 3 and q == 1) printf("   ");
            }
        }        
    }
}

nats double_odd(nat* h, nat m, nat n, nat i, nat l) {
    nats neighbors = {i};        
    for (int j = 1; j < l; j *= m) { 
        neighbors.push_back(i + j * ((i / j + 1) % m - i / j % m));
        neighbors.push_back(i + j * ((i / j + m - 1) % m - i / j % m));
    }
    return neighbors;
}

nats double_even(nat* h, nat m, nat n, nat i, nat l) {    
    nats neighbors = {};
    
    for (int j = 1; j < l; j *= m) {
        neighbors.push_back(i + j * ((i / j + 1) % m - i / j % m));
        neighbors.push_back(i + j * ((i / j + m - 1) % m - i / j % m));
    }
    return neighbors;
}

nats single_odd(nat* h, nat m, nat n, nat i, nat l) {    
    nats neighbors = {i};
    for (int j = 1; j < l; j *= m) {
        neighbors.push_back(i + j * ((i / j + 1) % m - i / j % m));        
    } 
    return neighbors; 
}

nats single_even(nat* h, nat m, nat n, nat i, nat l) {
    nats neighbors = {};
    for (int j = 1; j < l; j *= m) {
        neighbors.push_back(i + j * ((i / j + 1) % m - i / j % m));
        
    }
    return neighbors;
}

nat serialize_vsbn(nat* h, nat m, nat n, nat j, nat l) { /// converts the given global neighborhood into an index representing the local neighborhood.    
    auto neighbors = single_even(h, m, n, j, l);     
    return unreduce(neighbors, 0, m);
}

void initialize(nat* g, nat l) {
    memset(g, 0, sizeof(nat) * l);
    g[0] = 1;
}

nats add_mod_component_wise(nats a, nats b, nat m) {
    auto r = a;
    for (nat i = 0; i < a.size(); i++) {
        r[i] += b[i];
        r[i] %= m;
    }
    return r;
}

nat desitter_offset(nat point, nat offset, nat m, nat n) {
    
    auto point_r = reduce(point, m, n);
    //    std::cout << "\tjr = ";
    //    print(point_r);
    
    
    auto offset_r = reduce(offset, m, n);
    //    std::cout << "\tzr = ";
    //    print(offset_r);
    
    
    auto sum = add_mod_component_wise(point_r, offset_r, m);
    //    std::cout << "\tsum = "; print(sum);
    
    return unreduce(sum, m, n);
}

int main(int argc, const char * argv[]) {
    
    print_args(argv, argc);    
    
    if (argc < 7) {
        printf("usage: ./ads " // 0
               "M-nat0 " // 1
               "N-nat0 " // 2
               "t-nat0 " // 3
               "Thr-nat0 " // 4 
               "Nd-bool " // 5
               "NP-bool \n\n"); // 6
        exit(1);
    }    
    
    nat
    m = atoi(argv[1]),
    n = atoi(argv[2]),
    t = atoi(argv[3]),
    l = pow(m, n),
    H = pow(m, l),
    Z = pow(m, H);
    
    nat threshold = atoi(argv[4]);
    n_dimensional_display = atoi(argv[5]);
    numeric_print = atoi(argv[6]);
    
    std::vector<nat> lengths {};
    
    std::cout << "searching the computational universe...\n";
    
    for (nat z = 0; z < Z; z++) {
        printf("\r[ z = %lu  /  %lu ] ", z, Z);
        auto result = reduce(z, m, H);                
        nat g[l], h[l];
        initialize(g, l);
        std::vector<nats> states = {};
        for (nat i = 0; i < t; i++) {
            memcpy(h, g, sizeof h);
            auto v = vector(h, l);
            for (nat j = l; j--;) g[j] = result[unreduce(v, m, l)];                                   
            if (not contains(states, v)) states.push_back(v);
        }
        lengths.push_back(states.size());
    }
    
    
    std::cout << "finding CAs with sufficent lifetimes...\n";
    
    nats big_lengths = {};
    nats indicies = {};
    
    for (nat i = 0; i < lengths.size(); i++) {        
        if (lengths[i] >= threshold) {
            big_lengths.push_back(lengths[i]);
            indicies.push_back(i);
        }
    }
    
    std::cout << "there were " << big_lengths.size() << " automatas with a sufficent length.\n";
    std::cout << "CA timesteps\n\n";
    for (nat i = 0; i < indicies.size(); i++) {         
        std::cout << "\t [" << i << "]  ::  (" << indicies[i] << ") : ";
        print_raw(reduce(indicies[i], m, H));
        std::cout << " ---> " << big_lengths[i] << " timesteps\n";
    }
}

//            print(h, l, n, m, i, z);
//            usleep(delay);
