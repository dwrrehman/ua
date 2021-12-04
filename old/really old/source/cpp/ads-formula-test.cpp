//
//  main.cpp
//  ads-formula-test
//
//  Created by Daniel Rehman on 1907232.
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
bool use_0d_sbr = true;

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"

void clear_screen() {printf("\e[1;1H\e[2J");}

void display(size_t x, size_t m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    size_t color = (size_t)(24.0 * ratio); // 24 possible valid hues    
    color += 232;  // offset to get only the 24 graytone colors.  
    printf(bold_color "██" reset, color); 
    //printf(bold_color "##" reset, color);
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


void u(size_t m, size_t n) {
    size_t l=pow(m,n),g[l],h[l];
    memset(g,0,sizeof g);g[0]=1;memcpy(h,g,sizeof h);
    
    for(size_t i=l;i--;memcpy(h,g,sizeof h)) {
        for(size_t j=l;j--;g[j]=(g[j]+1)%m)
            for(size_t k=l;k--;)for(size_t f=1;f<l;f*=m)
                g[j]=(g[j]+h[k]*((j/f+k/f))*((j/f+k/f)))%m;
        print(g, l, n, m, i);
    }
    /// old summtation: code:
    ///   g[j]+=h[k]*(j+k)*(j+k)/(f*f);
} 

void print_args(const char** argv, int argc) {
    printf("[");
    for (int i = 0; i < argc; i++) {
        printf("\t# %d: \"%s\"\n", i, argv[i]);
    }
    printf("]\n");
}


void give_me_1d_weights(size_t m) {
        
    size_t g[m];        
    for (size_t k = 0; k < m; k++) {
        size_t d = 0;
        for (size_t f = 1; f < m; f *= m) {
            d = (d + (k / f) * (k / f) ) % m;
        }
        g[k] = d % m;         
    }    
    print(g, m, 1, m, 0);
    exit(0);
}





int main(int argc, const char * argv[]) {
    
    
    print_args(argv, argc);
    
    if (argc < 7) {
        printf("usage: ./ads M-nat0 N-nat0 delay-nat Nd-bool 0d-bool timesteps-nat\n\n");
        exit(1);
    }
    
    size_t 
    m = atoi(argv[1]), 
    n = atoi(argv[2]), 
    l = pow(m, n), g[l], h[l];
    int delay = atoi(argv[3]);
    n_dimensional_display = atoi(argv[4]);
    use_0d_sbr = atoi(argv[5]);
    int t = atoi(argv[6]);
    memset(g, 0, sizeof g);
    g[0] = 1;

    for (size_t i = 0; i < t; i++) {        
        memcpy(h, g, sizeof h);        
        for (size_t j = l; j--;) {            
            for (size_t k = l; k--;) {
                for (size_t f = 1; f < l; f *= m) {
                    g[j] = (g[j] + h[k] * ((j/f + k/f)) * ((j/f + k/f))) % m;
                }
            }            
            //for (size_t f = 1; f < l; f *= m)
                g[j] = (g[j]) % m;  // somethign with f.                                       //not use_0d_sbr or g[j]
        }
        

        if (n_dimensional_display) clear_screen();
        print(h, l, n, m, i);
        usleep(delay);
    }
 
    return 0;
}





//    std::ofstream image{"picture.ppm", std::ofstream::trunc}; 
//    image << "P3\n";
//    image << l << " " << m << "\n"; // W, H.
//    image << "255" << "\n";
//       


//        for (int k = 0; k < l; k++) {
////            auto e = h[k] % m;
////            const double ratio = (double) e / (double) m;
////            const size_t v = ratio * 255;
//            //image << v << " " << v << " " << v << "\n";
//        }

//image.close();
//system("open picture.ppm");
