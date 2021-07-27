#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>


int are_equal(int* des, int* ns, long l) {
    for (int i = 0; i < l; i++) {
        if (des[i] != ns[i]) return 0;
    }
    return 1;
}

static int special_case(int i, int m, int n, int l, int* u, int* w) {
    
    int ns[2 * n + 1];
    memset(ns, 0, sizeof ns);
    ns[0] = w[i];
    int o = 1;
    
    for (int j = 1, k = i + m; j < l; j *= m, k = i / j + m) { 
        ns[o++] = w[i + j * ((k + 1) % m - k % m)];
        ns[o++] = w[i + j * ((k - 1) % m - k % m)];
    }
    
    int des0[5] = {0, 0, 0, 0, 1};
    int des1[5] = {0, 0, 1, 1, 1};
    return are_equal(des0, ns, 5) || are_equal(des1, ns, 5);     
}

void print(int* u, int l) {
    for (int i = 0; i < l; i++)
        printf(" %c ", u[i] ? '0' : ' '); 
    puts("\n"); usleep(100000);
}

int main() {
    int m = 20, n = 2, l = pow(m,n), w[l], u[l];
    memset(u, 0, sizeof u);
    u[0] = 1;
    for(int t = l; t--;) {
        memcpy(w, u, sizeof w);
        for (int i = l; i--;) {
            for (int j = 1, k = i; j < l; j *= m, k = i / j)
                u[i] += w[i + j * ((k + 1) % m - k % m)] + w[i + j * ((k + m - 1) % m - k % m)];
            u[i] = u[i] && u[i] < 2 * n + 1 && !0; // unfinsihed.
        } 
        print(u, l);
    }
    return 0;
}


/*

// more polished version:

void a(int m, int n) {
    int l = pow(m,n), w[l], u[l];
    memset(u, 0, sizeof u); u[0] = 1;
    for (int _ = l; _--;) {
        memcpy(w, u, sizeof w);
        for (int i = l; i--;) {
            for (int j = 1, k = i + m; j < l; j *= m, k = i / j + m) 
                u[i] += w[i + j * ((k + 1) % m - k % m)] + w[i + j * ((k - 1) % m - k % m)]; // we really need to make this line of code simpler, and also shorter.
            for (int j = 0; j < n; j++)
                u[i] = u[i] && u[i] < 2 * n + 1 && !u[j]; // unfinished: must implement Nd SB cases generally.
        }
    }
}


 
    // (something like) the dream:
     
int main() {
    int m = 20, n = 2, l = pow(m, n), w[l], u[l];
    memset(u, 0, sizeof u); u[0] = 1;
    for (int _ = l; _--;) {
        memcpy(w, u, sizeof w);
        for (int i = l; i--;) {
            for (int j = n; j--;) u[i] += w[(i + 1) % m] + w[(i - 1) % m];          // isnt this one nicer?
            for (int j = n; j--;) u[i] = u[i] && u[i] < 2 * n + 1 && !sc;           // what if these two lines could merge! wouldnt that be awesome?             i doubt it though.
        }
    }
}
 
    





// special case for n = 1:   u[i] == 1 && w[i + (i + 1) % m - i % m]

/// junk: //int c = u[i] == 1 && (w[i + (i + 1) % m - i % m] && w[i + m * ((i / m + 1) % m - i / m % m)]);

///return u[i] == 2 && ((w[i + (i + 1) % m - i % m] && w[i + m * ((i / m + 1) % m - i / m % m)]) || (w[i + (i + 1) % m - i % m] && w[i + m * ((i / m - 1) % m - i / m % m)]));


//for (int q = 1; q < n; q++) if (i % (int)pow(m,q) == (int)pow(m,q) - 1) printf("\n");
*/





/* ---------------- new stuff! ---------------

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>


void print(size_t* u, size_t s) {
    for (int i = 0; i < l; i++) printf(" %c ", u[i] ? '0' : ' ');
    puts("\n"); usleep(100000);
}

int main() {
    const size_t m = 10, n = 1, s = m * m, l = pow(s,n);
    size_t g[s], h[s];
    memset(g, 0, sizeof g);
    g[0] = 1;
 
    for (size_t i = 0; i < l; i++) {
        memcpy(h, g, sizeof h);
        for (size_t j = 0; j < s; j++) {
            for (size_t k = 1, f = j; k < S; k *= s, f = j / k) {
                g[i] += h[j + k * ((f + 1) % s - f % s)]
                + h[j + k * ((f + s - 1) % s - f % s)];
            }
            g[j] = 0;
        }
        print(h, s);
    }
    return 0;
}
*/
