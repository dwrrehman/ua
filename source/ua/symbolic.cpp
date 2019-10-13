//
//  symbolic.cpp
//  ua
//
//  Created by Daniel Rehman on 1910137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "symbolic.hpp"


#include "structures.h"
#include "utilities.hpp"
#include "parameters.hpp"

#include "io.hpp"
#include "ca_util.hpp"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


std::vector<token> lex(std::string input_string) {
    
    std::vector<token> tokens = {};    
    std::istringstream input {input_string};
    
    while (input.good()) {
        std::string s = "";

        input >> s;
        
        if (not input.good()) break;
        
        std::cout << "new token  -->  \"" << s << "\"\n";
        
        
        if (s == "+") tokens.push_back({expression_type::operator_, operator_type::add});
        
        else if (s == "-") tokens.push_back({expression_type::operator_, operator_type::subtract});
        
        else if (s == "*") tokens.push_back({expression_type::operator_, operator_type::multiply});
        
        else if (s == "=") tokens.push_back({expression_type::operator_, operator_type::equals});
                
        else if (s.back() == ']') {
            s.pop_back();
            s.erase(s.begin());
            nat n = std::stoi(s);
            std::cout << "[n] var = " << n << "\n";
            tokens.push_back({expression_type::variable, {}, n}); 
            
        } else if (s == "(") {
            tokens.push_back({expression_type::paren, {}, 0, true});
            
        } else if (s == ")") {
            tokens.push_back({expression_type::paren, {}, 0, false});
            
        } else {
            nat n = std::stoi(s);
            std::cout << "n lit. = " << n << "\n";
            tokens.push_back({expression_type::constant, {}, n});
        }
    }
    return tokens;
} 



std::string convert_type(enum expression_type type) {
    if (type == expression_type::null) return "null";
    else if (type == expression_type::operator_) return "operator";
    else if (type == expression_type::constant) return "constant";
    else if (type == expression_type::variable) return "variable";
    else if (type == expression_type::paren) return "paren";
    else abort();
} 


std::string convert_operator(enum operator_type type) {
    if (type == operator_type::null) return "null";
    else if (type == operator_type::add) return "add";
    else if (type == operator_type::multiply) return "multiply";
    else if (type == operator_type::subtract) return "subtract";
    else if (type == operator_type::equals) return "equals";
    else abort();
} 

#define prep(d) for (nat i = d; i--;) printf("   .")

void print_expression(expression e, nat d) {
    prep(d); std::cout << "error = " << std::boolalpha << e.error << "\n";    
    prep(d); std::cout << "expression type = " << convert_type(e.type) << "\n";    
    prep(d); std::cout << "operator type = " << convert_operator(e.operator_) << "\n";    
    prep(d); std::cout << "constant value = " << e.constant_value << "\n";    
    prep(d); std::cout << "variable index = " << e.variable_index << "\n";    
    prep(d); std::cout << "children count = " << e.children.size() << "\n";    
    if (e.children.size()) {
        prep(d); std::cout << "children: \n";
        for (auto child : e.children) {
            print_expression(child, d + 1); 
        }
    }
    std::cout << "\n";
}



void print_parse(expression e) {
    std::cout << "-------- printing parse: ------- \n";
    print_expression(e, 0);
}


std::string stringify(expression e) {
    if (e.type == expression_type::constant) return  "( " + std::to_string(e.constant_value) + " )";
    else if (e.type == expression_type::variable) return "( [" + std::to_string(e.variable_index) + "] )";    
    else if (e.type == expression_type::operator_) {
        if (e.operator_ == operator_type::add) return "( " + stringify(e.children[0]) + " + " + stringify(e.children[1]) + " )";
        if (e.operator_ == operator_type::subtract) return "( " + stringify(e.children[0]) + " - " + stringify(e.children[1]) + " )";
        if (e.operator_ == operator_type::multiply) return "( " + stringify(e.children[0]) + " * " + stringify(e.children[1]) + " )";
        if (e.operator_ == operator_type::equals) return "( " + stringify(e.children[0]) + " = " + stringify(e.children[1]) + " )";
    }
    return "{INTERNAL ERROR}";
} 


void print_lex(std::vector<token> tokens) {
    std::cout << "---------- printing lex: --------\n";
    for (auto t : tokens) { 
        std::cout << "\ttoken type = " << convert_type(t.type) << "\n";
        std::cout << "\toperator type = " << convert_operator(t.operator_) << "\n";
        std::cout << "\tvalue = " << t.value << "\n";
        std::cout << "\topen = " << std::boolalpha << t.open << "\n";
        std::cout << "\n";
    }
    std::cout << "end of lex.\n";
}


expression parse_expression(std::vector<token> tokens);

nat pointer = 0;
nat save() { return pointer; }
void revert(nat saved) { pointer = saved; }

const expression error = {{}, 0, 0, {}, {}, true};


expression parse_operator(std::vector<token> tokens, operator_type given_operator) {
        
    auto saved = save();
    
    auto left = parse_expression(tokens);
    if (left.error) {
        revert(saved);
        return error;
    }
    
    auto t = tokens[pointer++];    
    if (not (t.type == expression_type::operator_ and 
             t.operator_ == given_operator)) {
        revert(saved);    
        return error;
    }
    
    auto right = parse_expression(tokens);
    if (right.error) {
        revert(saved);
        return error;
    }
        
    return expression {
        expression_type::operator_,
        0, 0, given_operator,
        {left, right}
    };
}


expression parse_variable(std::vector<token> tokens) {
    auto saved = save();
    auto t = tokens[pointer++];
    if (t.type == expression_type::variable) 
        return expression {expression_type::variable, 0, t.value};    
    else {
        revert(saved);    
        return error;
    }
}

expression parse_constant(std::vector<token> tokens) {
    auto saved = save();
    auto t = tokens[pointer++];
    if (t.type == expression_type::constant) 
        return expression {expression_type::constant, t.value};    
    else {
        revert(saved);    
        return error;
    }
}


expression parse_expression(std::vector<token> tokens) {
    auto saved = save();
    
    expression e;
    e.error = true;
    
    auto t = tokens[pointer++];    
    if (not (t.type == expression_type::paren and t.open)) {
        revert(saved);
        return error;
    }
    
    if (e.error) e = parse_operator(tokens, operator_type::equals);
    if (e.error) e = parse_operator(tokens, operator_type::add);
    if (e.error) e = parse_operator(tokens, operator_type::subtract);
    if (e.error) e = parse_operator(tokens, operator_type::multiply);
    if (e.error) e = parse_constant(tokens);
    if (e.error) e = parse_variable(tokens);
    
    t = tokens[pointer++];
    if (not (not e.error and t.type == expression_type::paren and not t.open)) {
        revert(saved);
        return error;
    } else return e;
}

expression parse(std::vector<token> tokens) {        
    return parse_expression(tokens);
}

expression compile(std::string input) {    /// [1] + ( [3] * 2 = ( 4 ) )
    return parse(lex(input));
}


nat evaluate(const expression& given, neighborhood ns, const nat m) {
    
    if (given.type == expression_type::constant)
        return given.constant_value;
    
    else if (given.type == expression_type::variable) 
        return ns[given.variable_index];
    
    else if (given.type == expression_type::operator_) {        
        if (given.operator_ == operator_type::add) 
            return (evaluate(given.children[0], ns, m) + 
                    evaluate(given.children[1], ns, m)) % m;
        
        else if (given.operator_ == operator_type::subtract) 
            return (evaluate(given.children[0], ns, m) - 
                    evaluate(given.children[1], ns, m) + m) % m;
        
        else if (given.operator_ == operator_type::multiply) 
            return (evaluate(given.children[0], ns, m) *
                    evaluate(given.children[1], ns, m)) % m;
        
        else if (given.operator_ == operator_type::equals) 
            return evaluate(given.children[0], ns, m) == evaluate(given.children[1], ns, m);
    } 
    
    if (given.type == expression_type::null) {
        std::cout << "trying to execute a null instruction... aborting.\n";
        print_expression(given, 0);        
    }
    abort();
}

