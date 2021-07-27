#include <iostream>
#include <vector>
#include <math.h>
using namespace std;

int i(vector<int> u, vector<int> v, int n, int m) {
    int c = 0;
    for (int i = 0; i < n; i++)
        c += v[i] * pow(m, i);
    return u[c];
}

void print(std::vector<int> space) { 
    std::cout << space.size() << ": ";
    for (auto unit : space) {
        std::cout << "[" << unit << "]";
    } std::cout << "\n";
}

static void init(int l, vector<int> &u) {
    for (int i = 0; i < l; i++) {
        u[i] = i;
    }    
}

static void print_neighbors(int s, vector<int> neighbors) {
    std::cout << "h("<< s <<") = { ";
    for (auto neighbor : neighbors) {
        std::cout << neighbor << " ";         
    } std::cout << "}\n";
}

static bool isin(int e, vector<int> v) {
    for (auto s : v) {
        if (s == e) return true;
    }
    return false;
}

static void pretty_print_neighbors(int s, vector<int> neighbors, int l, int m, int n) {
    std::cout << "\n";
    for (int i = 0; i < l; i++) {
        if (i == s) std::cout << "[X]";
        else if (isin(i, neighbors)) {
            std::cout << "[o]";
        } else std::cout << "[ ]";
        for (int q = 1; q < n; q++) 
            if (i % (int)pow(m,q) == (int)pow(m,q) - 1) 
                std::cout << "\n";                    
    }
    std::cout << "\n";
}

void box() {
    std::cout << "box:\n\n\t[0][1][2][3]\n\t[4][5][6][7]\n\t[8][9][A][B]\n\t[C][D][E][F]\n\n";
}






void experiment(vector<int> u, int l, int n, int m) {
        
    printf("printing v's based on s:\n");
    
    for (int i = 0; i < l; i++) {
    
        vector<int> ns = {}; // size: 2n     /// FUNDEMENTAL        
        for (int g = 0; g < n; g++) {
            int s = 0, z = 0;
            for (int j = 0, p = 1; j < n; j++, p *= m) {
                s += p * ((i/p + 1       * (g == j)) % m);
                z += p * ((i/p + (m - 1) * (g == j)) % m);
            }
            ns.push_back(u[s]);
            ns.push_back(u[z]);
        }            
        
        print_neighbors(u[i], ns);
        pretty_print_neighbors(u[i], ns, l, m, n);
    }
          
    exit(0);
}

int main() {
    
    int m = 3, n = 3, l = pow(m, n);
    vector<int> u(l, 0);
    init(l, u);
    
    for (int i = 0; i < l; i++) {
        vector<int> ns = {};
        
        for (int j = 1; j < l; j *= m) {
            
            int a = ((i / j + 1) % m - i / j % m);
            printf("a = %d\n", a);
            
            int ap = ((i / j + 1) % m - i / j % m);
            printf("ap = %d\n", ap);
            
            ns.push_back(u[i + j * ((i / j + 1) % m - i / j % m)]);
            ns.push_back(u[i + j * ((i / j + m - 1) % m - i / j % m)]);
        }
        
        print_neighbors(u[i], ns);
        pretty_print_neighbors(u[i], ns, l, m, n);
    }
    
    printf("%d\n", (-47)%3);
} 
