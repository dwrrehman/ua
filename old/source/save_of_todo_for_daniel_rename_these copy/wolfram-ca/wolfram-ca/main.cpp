//
//  main.cpp
//  wolfram-ca
//
//  Created by Daniel Rehman on 1906171.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

size_t M = 300;
uint S = 50000; // the speed of light, sort of.

bool quit = false;
using space = std::vector<size_t>;

void initialize(space& space) {
    space = std::vector<size_t>(M, 0);
    space[M / 2] = 1;
}

size_t rule(size_t a, size_t b, size_t c) { /// Wolfram CA Rule 110.
    if (a and b and c)      return 0;
    if (a and b and !c)     return 1;
    if (a and !b and c)     return 1;
    if (a and !b and !c)    return 1;
    if (!a and b and c)     return 1;
    if (!a and b and !c)    return 1;
    if (!a and !b and c)    return 0;
    if (!a and !b and !c)   return 0;
    return 0;
}

/// if ((a+b+c == 1 or a+b+c == 2) and not (a and !b and !c)) return 1; else return 0;

void evolve(space& current) {
    space future = current;
    for (size_t x = 0; x < M; x++) {
        const auto me = current[x];
        const auto left = current[(x - 1) % M];
        const auto right = current[(x + 1) % M];
        future[x] = rule(left, me, right);
    }
    current = future;
}

void input() {
    char c = 0;
    while (c != 'q') std::cin >> c;
    quit = true;
}

void print_space(space space) {
    std::cout << "  ";
    for (auto unit : space) {
        if (unit) std::cout << "█ ";
        else std::cout << "  ";
    }
    std::cout << "  \n";
}

static void simulate(space& space) {
    while (!quit) {        
        print_space(space);
        evolve(space);
        usleep(S);
    }
}

int main(int argc, char** argv) {
    if (argc < 3) exit(1);
    M = atoi(argv[1]);
    S = atoi(argv[2]);    
    space space = {};
    initialize(space);            
    std::thread input_thread(input);    
    simulate(space);
    input_thread.join();
}
