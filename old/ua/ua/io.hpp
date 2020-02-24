//
//  io.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef io_hpp
#define io_hpp


#include "utilities.hpp"
#include <vector>

char getch();

void print(std::vector<nat> v, const char* message = "", bool raw = false);
void print(std::vector<nat> h, nat timestep, const parameters& u);

void write_scores(const std::vector<score>& scores, const parameters& u);
std::vector<nat> read_nats(std::string source);
void print_results(const std::vector<score>& scores, const parameters& u);

#endif /* io_hpp */
