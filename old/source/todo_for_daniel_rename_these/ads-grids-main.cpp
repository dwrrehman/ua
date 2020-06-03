//
//  main.cpp
//  ads-grids
//
//  Created by Daniel Rehman on 1907206.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <vector>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define normal_color "\033[38;5;%lum"
#define bold_color "\033[1;38;5;%lum"
#define reset "\033[0m"



void clear_screen() {printf("\e[1;1H\e[2J");}



using Color = long int;
using nat = size_t;
using grid = std::vector<std::vector<nat>>;


void display(nat x, nat m) {                      // display a modnat(m) as a block of a particular graytone hue.
    double ratio = (double) x / (double) m;    
    Color color = (long int)(24.0 * ratio); // 24 possible valid hues    
    color += 232;  // offset to get only the 24 graytone colors.  
    printf(bold_color "██" reset, color);
    //printf(bold_color "##" reset, color);
}


void numeric_print(grid grid) {
    std::cout << "grid (" << grid.size() * grid.size() << "):\n";    
    auto j = 0;
    std::cout << "\n       ";
    for (auto i = 0; i < grid.size(); i++) std::cout << " " << std::setw(4) << i << " "; 
    std::cout << "\n       ";
    for (auto i = grid.size(); i--;) std::cout << "------";
    std::cout << "\n";
    for (auto x : grid) {        
        std::cout << " " << std::setw(4) << j++ << " |";
        for (auto y : x) {
            std::cout << " " << std::setw(4) << y << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void color_print(grid grid) {
    std::cout << "grid (" << grid.size() * grid.size() << "):\n";    
    auto j = 0;
    
    std::cout << "\n       ";
    for (auto i = 0; i < grid.size(); i++)  std::cout << " " << std::setw(2) << i;
    
    std::cout << "\n       ";
    for (auto i = grid.size(); i--;) std::cout << "---";
    
    std::cout << "\n";
    
    for (auto x : grid) {        
        std::cout << " " << std::setw(4) << j++ << " |";
        for (auto y : x) {
            std::cout << " ";            
            display(y, grid.size());            
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}



/*
 observations:
 
    i, j, x, y are all mod m.               (ie, they are in the LRS(M))
 
    
 */
//nat distance(nat m,         // modulus.
//             nat x, nat y,  // origin.
//             nat i, nat j   // new point.
//             ) {
//                 return ((i + x) * (i + x) + (j + y) * (j + y) + 1) % m;
//}

//void fill_with_weights_from(grid& grid, nat x, nat y, nat M, nat t) {
//    for (auto i = 0; i < M; i++)
//        for (auto j = 0; j < M; j++)
//            
//}



void fill_with_weights_from(grid& grid, nat x, nat y, nat M) {      // mod M desitter/ads-space distance, i think....
    for (auto i = 0; i < M; i++) 
        for (auto j = 0; j < M; j++) 
            grid[i][j] = ((i + x) * (i + x) + (j + y) * (j + y)) % M;
}



int main() {
    
    nat M = 1;
    
    while (M) {
        
        std::cout << "give M: ";    
        std::cin >> M;
        
        size_t cp = 0;
        std::cout << "colorprint?(1/0): "; 
        std::cin >> cp; 
        
        grid grid = {M, std::vector<nat> (M, 0)};    
        if (cp) color_print(grid);
        else numeric_print(grid);
        
        std::cout << "pick a point (v h): "; 
        nat x = 0, y = 0;
        std::cin >> x;
        std::cin >> y;        
      
        clear_screen();
        fill_with_weights_from(grid, x, y, M);                        
        if (cp) color_print(grid);
        else numeric_print(grid);
    }
    
    
}


//    int delay = 0;
//    std::cout << "give usleep: ";
//    std::cin >> delay;
//  
