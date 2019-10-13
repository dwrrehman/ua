//
//  io.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "io.hpp"

#include "structures.h"
#include "utilities.hpp"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

char getch() {
    struct termios t = {0}; if (tcgetattr(0, &t) < 0) perror("tcsetattr()");
    t.c_lflag &= ~ICANON; t.c_lflag &= ~ECHO; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t) < 0) perror("tcsetattr ICANON");
    char c = 0; if (read(0, &c, 1) < 0) perror("read()"); t.c_lflag |= ICANON; t.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &t) < 0) perror("tcsetattr ~ICANON");
    return c;
}


void print(vec v, const char* message, bool raw) {
    std::cout << message << "[ ";
    for (auto e : v) std::cout << e << " ";
    std::cout << "]";
    if (not raw) std::cout << std::endl; 
}

static inline void clear_screen() {printf("\e[1;1H\e[2J");} 


#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset_color "\033[0m"

void print(vec h, nat timestep, const parameters& u) {
    if (u.n_dimensional_display) clear_screen();    
    else printf("%5lu:   ", timestep); 
    for (nat i = 0; i < u.L; i++) {
        if (u.numeric_print) printf("%4lu ", h[i]);
        else printf(bold_color "██" reset_color, (nat)(24.0 * (double) h[i] / (double) u.m) + 232);        
        if (u.n_dimensional_display) {
            for (nat q = 1; q < u.n; q++) {
                nat d = pow(u.size, q);
                if (i % d == d - 1) {
                    if (u.n == 2 or q == 2) printf("\n");
                    if (u.n == 3 and q == 1) printf("   ");
                }
            }             
        }
    }
    std::cout << std::endl;
}

void write_scores(const std::vector<score>& scores, const parameters& u) {    
    std::ofstream file {u.destination, std::ios_base::trunc};
    std::cout << "\n";
    for (nat i = 0; i < scores.size(); i++) {
        file << scores[i].z << " " << scores[i].score << "\n";
        printf("\r\twriting [ z = %lu  /  %lu ] : %lu ",  i, scores.size(), scores[i].z);
        usleep(10000);
    }
}

std::vector<nat> read_nats(std::string source) {
    std::ifstream file {source};    
    std::vector<nat> z_values = {};
    
    while (file.good()) {        
        nat z = 0;
        double score = 0.0;        
        file >> z;
        file >> score;        
        z_values.push_back(z);
    }
    return z_values;    
}

void print_results(const std::vector<score>& scores, const parameters& u) {
    std::cout << "\n\n\n------------------------------\n\n";    
    for (nat i = 0; i < scores.size(); i++) {
        printf("\t [ %lu ]  ::  ( %lu ) ---> %lf\n", i, scores[i].z, scores[i].score);
        std::vector<nat> H_grid(u.H, 0);
        reduce(H_grid, scores[i].z, u.m, u.H);
        print(H_grid, "\treduced:");
    }
    std::cout << "there were " << scores.size() << " automatas with a sufficent length.\n";
}
