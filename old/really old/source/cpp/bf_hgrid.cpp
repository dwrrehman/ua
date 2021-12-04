//
//  main.cpp
//  bf-hgrid
//
//  Created by Daniel Rehman on 1908051.
//

#include <iostream>
#include <vector>

/// types:

using nat = size_t;
using ab_t = std::pair<nat, nat>;     // [A, B] is a solution to     (i + A) * (j + B) = z[i][j]         where z is H grid solution matrix
using set = std::vector<ab_t>;    // a set of solutions.
using matrix_set = std::vector<std::vector<set>>;    // a matrix of sets of solutions.
using matrix = std::vector<std::vector<nat>>;

// printers:

void print_raw(std::vector<nat> v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
}

void print(std::vector<nat> v) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}


void print(std::vector<std::vector<nat>> z) {
    std::cout << "[\n";
    for (auto c : z) {
        std::cout << "\t";
        print(c);
    }
    std::cout << "]\n";
}

void print_ab(ab_t s) {
    std::cout << "[" << s.first << " " << s.second << "]";
}

void print_solutions(matrix_set solutions) {
    std::cout << "([me] [R]) :: {[A, B], ...} = [\n\n";
    for (nat i = 0; i < solutions.size(); i++) {
        for (nat j = 0; j < solutions.size(); j++) {
            std::cout << "\t[" << i << "] [" << j <<"] = { ";
            for (auto s : solutions[j][i]) {
                print_ab(s); std::cout << ", ";
            }
            std::cout << "}\n";
        }
        std::cout << "\n";
    }
    std::cout << "]\n";    
}

/// solvers:

matrix_set create_zero_solutions(matrix z) {
    std::vector<std::vector<set>> s {};
    s.resize(z.size(), {});        
    for (auto& e : s) {
        e.resize(z.size(), {});        
    }    
    return s;
}

bool satifies(nat i, nat j, nat a, nat b, nat c, nat m) {
    return ((i + a) % m) * ((j + b) % m) % m == c;
}

matrix_set solve(matrix z, nat m) {
    auto solution = create_zero_solutions(z);    
    for (nat i = 0; i < z.size(); i++) 
        for (nat j = 0; j < z.size(); j++)
            for (nat a = 0; a < z.size(); a++) 
                for (nat b = 0; b < z.size(); b++) 
                    if (satifies(i, j, a, b, z[i][j], m)) 
                        solution[i][j].push_back({a, b});
    return solution;
}


int main() {    

    const nat m = 3; // the modulus.
    
    const matrix z1 = {
        {2, 1, 0},
        {0, 1, 2},
        {1, 1, 0}
    };
    
    const matrix z2 = {
        {1, 0, 1},
        {2, 1, 1},
        {0, 2, 0}
    };
    
    auto s = solve(z2, m);
    print_solutions(s);
}
