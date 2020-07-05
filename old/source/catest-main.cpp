//
//  main.cpp
//  catest
//
//  Created by Daniel Rehman on 1907232.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <string.h>
#include <stdio.h>

bool n_dimensional_display = false;
bool numeric_print = false;

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"


using Color = long int;
using nat = size_t;
using grid = std::vector<std::vector<nat>>;

void clear_screen() {printf("\e[1;1H\e[2J");}

void display(nat x, nat m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    Color color = (long int)(24.0 * ratio); // 24 possible valid hues    
    color += 232;  // offset to get only the 24 graytone colors.  
    printf(bold_color "█" reset, color); 
    //printf(bold_color "##" reset, color);
}

void print(int* u, int l, int n, int m) {    
    for (int i = 0; i < l; i++) {
        if (numeric_print) printf("%4d ", u[i]);
        else display(u[i], m);
        if (n_dimensional_display) {
            for (int q = 1; q < n; q++) {
                int d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    }
    printf("\n");
}

//int main(int argc, const char* argv[]) {
//    if (argc < 2) exit(1);
//    int m = atoi(argv[1]), n = atoi(argv[2]), l = pow(m,n), r[l], s[l];
//    memset(r, 0, sizeof r);
//    r[0] = 1;
//    for (int i = l; i--;) {
//        print(r, l, n, m);
//        memcpy(s, r, sizeof s);
//        for (int j = l; j--;) {
//            for (int k = l; k--;)
//                for (int p = 1; p < l; p *= m)
//                    r[j] += s[k] * (j + k) * (j + k) / (p * p);
//            r[j] = (r[j] + 1) % m;
//        }
//    }
//}

void u(int m, int n) {
    int l=pow(m,n),g[l],h[l];
    memset(g,0,sizeof g);g[0]=1;memcpy(h,g,sizeof h);
    
    for(int i=l;i--;memcpy(h,g,sizeof h))
        for(int j=l;j--;g[j]=(g[j]+1)%m)
            for(int k=l;k--;)
                for(int f=1;f<l;f*=m)
                    g[j]=(g[j]+h[k]*((j/f+k/f))*((j/f+k/f)))%m;
    
    
    /// old summtation: code:
    ///   g[j]+=h[k]*(j+k)*(j+k)/(f*f);
} 

// yay 5 lines of code

void universe() {
    int m = 5918472, n = 6, l = pow(m,n), g[l], h[l]; memset(g, 0, sizeof g); g[0] = 1;
    for (int i = l; i--;) {
        memcpy(h, g, sizeof h);
        for (int j = l; j--;) {
            for (int k = l; k--;)
                for (int f = 1; f < l; f *= m)
                    g[j] += h[k] * (j + k) * (j + k) / (f * f);
            g[j] = (g[j] + 1) % m;
        }
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 2) exit(1);
    int m = atoi(argv[1]), n = atoi(argv[2]), l = pow(m,n), g[l], h[l];
    memset(g, 0, sizeof g);
    g[0] = 1;
    for (int i = l; i--;) {
        print(g, l, n, m);
        memcpy(h, g, sizeof h);
        for (int j = l; j--;) {
            for (int k = l; k--;)
                for (int f = 1; f < l; f *= m)
                    g[j] += h[k] * (j + k) * (j + k) / (f * f);
            g[j] = (g[j] + 1) % m;
        }
    }
}
