//
//  symbolic.hpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef symbolic_hpp
#define symbolic_hpp


#include "utilities.hpp"
#include "parameters.hpp"

#include "io.hpp"
#include "ca_util.hpp"

#include <vector>



std::string convert_type(enum expression_type type);
std::string convert_operator(enum operator_type type);
void print_expression(expression e, nat d);
void print_parse(expression e);
std::string stringify(expression e);
std::string pretty_stringify(expression e);
void print_lex(std::vector<token> tokens);


std::vector<token> lex(std::string input_string);
expression parse_operator(std::vector<token> tokens, operator_type given_operator);
expression parse_variable(std::vector<token> tokens);
expression parse_constant(std::vector<token> tokens);

expression parse_expression(std::vector<token> tokens);
expression parse(std::vector<token> tokens);
expression compile(std::string input);
nat evaluate(const expression& given, std::vector<nat> ns, const nat m);

#endif /* symbolic_hpp */
