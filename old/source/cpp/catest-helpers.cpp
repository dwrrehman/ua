//
//  helpers.cpp
//  catest
//
//  Created by Daniel Rehman on 1907232.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "helpers.hpp"


//void numeric_print(grid grid) {
//    std::cout << "grid (" << grid.size() * grid.size() << "):\n";    
//    auto j = 0;
//    std::cout << "\n       ";
//    for (auto i = 0; i < grid.size(); i++) std::cout << " " << std::setw(4) << i << " "; 
//    std::cout << "\n       ";
//    for (auto i = grid.size(); i--;) std::cout << "------";
//    std::cout << "\n";
//    for (auto x : grid) {        
//        std::cout << " " << std::setw(4) << j++ << " |";
//        for (auto y : x) {
//            std::cout << " " << std::setw(4) << y << " ";
//        }
//        std::cout << "\n";
//    }
//    std::cout << std::endl;
//}
//
//void color_print(grid grid) {
//    std::cout << "grid (" << grid.size() * grid.size() << "):\n";    
//    auto j = 0;
//    
//    std::cout << "\n       ";
//    for (auto i = 0; i < grid.size(); i++)  std::cout << " " << std::setw(2) << i;
//    
//    std::cout << "\n       ";
//    for (auto i = grid.size(); i--;) std::cout << "---";
//    
//    std::cout << "\n";
//    
//    for (auto x : grid) {        
//        std::cout << " " << std::setw(4) << j++ << " |";
//        for (auto y : x) {
//            std::cout << " ";            
//            display(y, grid.size());            
//        }
//        std::cout << "\n";
//    }
//    std::cout << std::endl;
//}
