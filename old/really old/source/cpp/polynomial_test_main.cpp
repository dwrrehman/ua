//
//  main.cpp
//  sandbox14
//
//  Created by Daniel Rehman on 2001271.
//                                                       
//
#include <iostream>
using nat = unsigned long long;
  
/// (2,1) equation:
/// (((0 + 1) * l + 1) * c + 1) * r + 0

/// r110eq has degree, p,   p = 4.
///
///  C[4] = {0, 1, 1, 0};
///  V[4] = {garbage, 2, 0, 1}

// returns the value of a polynomial function given by coeff C[], and variables V[],   using a given neighborhood state.
// (Evaluates using Horner's method).

///  k = 0;
///  for i in n {
///      k = k * h[V[i]] + C[i];
///  }
///  ret k;

nat eval(const nat* C, const nat* V, const nat* h, nat n) {
    nat k = 0;
    
    for (nat i = 0; i < n; i++) {
        k *= h[V[i]];
        k += C[i];
    }
    return k;
}

int main(int argc, const char * argv[]) {

    const nat m = 2;
    const nat n = 1;
    const nat k = 2 * n + 1;
    const nat p = 4;           // the degree of the equation.           /// p ≥ 2n + 1.
    
    const nat C[p] = {0, 1, 1, 0};   /// coefficients, read inside out.
    const nat V[p] = {0, 2, 0, 1};   /// variables, read inside out. V[i] ranges in k.
    const nat h[k] = {0, 0, 0};      /// neighborhood state:       (using natural ordering: C, R, L, ...)
    
    std::cout << (eval(C,V,h,p) + h[0]) % m << std::endl;   // purely additive nonshifting automata.
}
