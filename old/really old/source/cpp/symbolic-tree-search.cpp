//
//  main.cpp
//  symbolic-tree-search
//
//  Created by Daniel Rehman on 1912286.
//                                                       
//
/**

later, we will add: variable, and mult.

thats it. (for a 3,1 ss)

*/




#include <unistd.h>
#include <iostream>
#include <vector>

using nat = unsigned long long;

const nat m = 3;
const nat n = 1;
const nat nc = 2 * n + 1;
const nat value_count = m + nc;

enum operator_type {add, multiply, operator_count, none};

struct node {
    nat value = 0;                      // can have the values 0 through (m + nc - 1).
    enum operator_type type = none;     // if none, then value is valid. else children is valid.
    std::vector<node> children = {};    // has width 2, iff type != none.
};

void print(node n) {
    if (n.children.empty() and (n.value < m or n.value > 800)) {
        printf("%llu", n.value);
    } else if (n.children.empty()) {
        printf("%c", "CLRUD"[n.value - m]);
    } else {
        printf("(");
        print(n.children[0]);
        if (n.type == add) printf(" + ");
        if (n.type == multiply) printf(" * ");
        print(n.children[1]);
        printf(")");
    }
}

void printn(node n) {
    print(n);
    std::cout << "\n";
}

void printnd(node n) {
    std::cout << "[debug]: ";
    print(n);
    std::cout << "\n";
}

node expand(node n, enum operator_type type, nat L, nat R, nat value) {
    n.value = value;
    n.type = type;
    n.children = {{L, none, {}}, {R, none, {}}};
    return n;
}

node retract(node n, nat v) {
    n.value = v;
    n.type = none;
    n.children = {};
    return n;
}

void increment(enum operator_type& t) { nat& nt = (nat&) t;nt++; }

void generate(node& n, node& root, nat d, const nat max_depth) {
    
    if (d >= max_depth) return;
    const auto save = n.value;
    for (enum operator_type op = add; op < operator_count; increment(op)) {
        for (nat left = 1; left < value_count; left++) {
            for (nat right = 1; right < value_count; right++) {
                n = expand(n, op, left, right, 999);
                printn(root);
                n = retract(n, save);
            }
        }
        for (nat left = 1; left < value_count; left++) {
            n = expand(n, op, left, 998, 997);
            generate(n.children[1], root, d + 1, max_depth);
            printn(root);
            n = retract(n, save);
        }
        n = expand(n, op, 996, 995, 994);
        for (auto& child : n.children) generate(child, root, d + 1, max_depth);
    }
}


/**
 void generate(node& n, node& k, nat d, const nat max_depth) {
     
     if (d >= max_depth) return;
     printn(k);
     
     const auto save = n.value;
     
     for (enum operator_type op = add; op < operator_count; increment(op)) {
         for (nat left = 1; left < value_count; left++) {
             for (nat right = 1; right < value_count; right++) {
                 n = expand(n, op, left, right, 0);
                 for (auto& child : n.children) generate(child, k, d + 1, max_depth);
                 n = retract(n, save);
             }
         }
     }
     
 }
 */

int main(int argc, const char * argv[]) {
    const nat max = 3;
    node root {0, none, {}};
    generate(root, root, 0, max);
    return 0;
}
