#include <math.h>
#include <stdio.h>
#include <string.h>

int main() {
    int m = 4, n = 0, l = pow(m,n), u[l], w[l];
    memset(u, 0, sizeof u); 
    u[0] = 1;
    for (int t = l; t--;) {
        memcpy(w, u, sizeof w);
        for (int i = l; i--;) {
            for (int j = l; j--;) {
                u[i] += w[j];
            }
            u[i] = (u[i] + 1) % m;                              
        }
    }
}
