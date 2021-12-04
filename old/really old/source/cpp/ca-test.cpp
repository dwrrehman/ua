//
//  main.cpp
//  ca-test
//
//  Created by Daniel Rehman on 1906285.
//                                                       
//

#include <iostream>
#include <vector>
#include <thread>

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;
bool contin = true;
bool quit = false;
bool should_stack = false;
int speed = 45000;

using rule = vector<int>; 

void print_ns(vector<int> ns, bool b) {
    if (!b) {
        printf("%d: [ ", (int)ns.size());
    } else if (ns.size() == 5) {
        std::cout << 16*ns[0] + 8*ns[1] + 4*ns[2] + 2*ns[3] + ns[4]; 
        std::cout << " --> [";
    }
    for (int i = 0; i < ns.size(); i++) {        
        printf("%d ", ns[i]);
    } printf("]\n");
}

void print(vector<int> u, int l, int n, int m) {
    
    for (int i = 0; i < l; i++) {
        
        if (u[i]) printf("0 ");
        else printf("  ");
        
        if (should_stack) {
            for (int q = 1; q < n; q++) {
                int d = pow(m,q);
                if (i % d == d - 1) {
                    if (n == 2 or q == 2) printf("\n");
                    
                    if (n == 3 and q == 1) printf("   ");
                }
            }
        }
    } printf("\n");
}

int are_equal(vector<int> des, vector<int> ns) {
    if (ns.size() != des.size()) return 0;
    for (int i = 0; i < ns.size(); i++) {
        if (des[i] != ns[i]) return 0;
    }
    return 1;
}

int special_case(int i, int m, int n, int l, vector<int> u, vector<int> w, rule rule1, rule rule2, rule rule3) {
    
    vector<int> ns(2 * n + 1, 0);    
    ns[0] = w[i];
    int o = 1;
    
    for (int j = 1, k = i + m; j < l; j *= m, k = i / j + m) { 
        ns[o++] = w[i + j * ((k + 1) % m - k % m)];
        ns[o++] = w[i + j * ((k - 1) % m - k % m)];
    }
    
    if (n == 1) return are_equal(rule1, ns);
    else if (n == 2) return are_equal(rule1, ns) || are_equal(rule2, ns);
    else if (n == 3) return are_equal(rule1, ns) || are_equal(rule2, ns) || are_equal(rule3, ns);
    else return false;
}

static void ua(int l, int m, int n, vector<int> u, vector<int> w, rule rule1, rule rule2, rule rule3) {
    printf("using rule1: ");
    print_ns(rule1, true);
    printf("and rule2: ");
    print_ns(rule2, true);
    printf("and rule3: ");
    print_ns(rule3, true);
    sleep(2);
    while (contin and !quit) {
        w = u;
        for (int i = l; i--;) {
            for (int j = 1, k = i + m; j < l; j *= m, k = i / j + m) 
                u[i] += w[i + j * ((k + 1) % m - k % m)] + w[i + j * ((k - 1) % m - k % m)];
            u[i] = u[i] && u[i] < 2 * n + 1 && !special_case(i, m, n, l, u, w, rule1, rule2, rule3); // there are exactly n + 2 special cases?
        }
        print(u, l, n, m);
        usleep(speed);
    }
}

void input() {
    char c = 0;
    while (c != 'q') {
        std::cin >> c;
        if (c == 'c' || c == ';' || c == '\'' || c == ']' || c == 'f') {
            contin = false;
        }
    }
    quit = true;
}

int main(int argc, const char** argv) {
    
    if (argc < 7) {
        std::cout << "please provide a M, N, delay, R1, R2, and R3. (nat from 0 to 31)\n\t[try \"./ca-test 50 2 70000 1 7\"?]\n";
        exit(1);
    }
    
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    speed = atoi(argv[3]);
    int rule1_start = atoi(argv[4]);
    int rule2_start = atoi(argv[5]);
    int rule3_start = atoi(argv[6]);
    
    int l = pow(m,n);
    vector<int> w(l, 0);
    vector<int> u(l, 0);
    u[0] = 1;
    
    int nidm = false;
    std::cout << "do you want to visualize N dimensionally? (0 / 1)";
    std::cin >> nidm;
    should_stack = nidm;
    
    std::cout << "starting at major,minor rule idx : (";
    std::cout << rule1_start << ", "<< rule2_start << ", " << rule3_start << ")...\n";
    sleep(3);
    
    vector<rule> rules1 = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1},
    };
    
    vector<rule> rules2 = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 1},
        {0, 0, 1, 1, 0},
        {0, 0, 1, 1, 1},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 1, 1, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 1, 1},
        {1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 1, 0},
        {1, 0, 1, 1, 1},
        {1, 1, 0, 0, 0},
        {1, 1, 0, 0, 1},
        {1, 1, 0, 1, 0},
        {1, 1, 0, 1, 1},
        {1, 1, 1, 0, 0},
        {1, 1, 1, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 1, 1, 1, 1},
    };
    
    std::vector<rule> rules3 = {
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 0, 1},
        {0, 0, 0, 0, 1, 1, 0},
        {0, 0, 0, 0, 1, 1, 1},
        {0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 1},
        {0, 0, 0, 1, 0, 1, 0},
        {0, 0, 0, 1, 0, 1, 1},
        {0, 0, 0, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 1},
        {0, 0, 0, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1},
        {0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 1},
        {0, 0, 1, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 1},
        {0, 0, 1, 0, 1, 0, 0},
        {0, 0, 1, 0, 1, 0, 1},
        {0, 0, 1, 0, 1, 1, 0},
        {0, 0, 1, 0, 1, 1, 1},
        {0, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 1},
        {0, 0, 1, 1, 0, 1, 0},
        {0, 0, 1, 1, 0, 1, 1},
        {0, 0, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 0, 1},
        {0, 0, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 1},
        {0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 1, 0},
        
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0},
    };
    
    vector<rule> rules = {};
    if (n == 1) rules = rules1;
    if (n == 2) rules = rules2;
    if (n == 3) rules = rules3;
    
    if (rules.empty()) {
        printf("error: no rules found for that dimensionality.\n"); 
        exit(1);
    }
    
    bool at_start = true;
    
    std::thread input_thread(input); 
    for (int i = rule1_start; i < rules.size(); i++) { 
        for (int j = rule2_start; j < rules.size(); j++) { 
            for (int k = rule3_start; k < rules.size(); k++) {                 
                contin = true;
                if (at_start) at_start = false;
                ua(l, m, n, u, w, rules[i], rules[j], rules[k]);             
                if (quit) break;                
            }
        }
    }
    input_thread.join();
}
