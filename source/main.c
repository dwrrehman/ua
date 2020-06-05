/// UA theory codebase,
/// used by and created
/// by daniel rehman
/// for experiments,
/// and testing.

#include "structures.h"
#include "parameters.h"
#include "vector.h"
#include "hgrid.h"
#include "ca.h"
#include "io.h"
#include "help.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const char* default_home = "/Users/deniylreimn/Documents/projects/ua/testing";


void load_file(char** input, nat count, struct context* context) {
    if (count == 1) return;
    else if (strings_equal(input[1], "param")) load_parameters(input[2], context);
    else if (strings_equal(input[1], "hgrid")) printf("...unimplemented.\n");
    else {
        printf("error: load: unknown file type: %s\n", input[2]);
        printf("available file types: \n"
               "\t param\n"
               "\t hgrid\n\n");
    }
}

void calculate_function(char** input, nat count, struct context context) {
    if (count == 1) return;
        
    else if (strings_equal(input[1], "reduce")) {
        element out[context.parameters.H];
        reduce(out, atoll(input[2]), context.parameters.m, context.parameters.H);
        print_vector_line_message("result = ", out, context.parameters.H);
        
    } else if (strings_equal(input[1], "unreduce")) {
        element in[context.parameters.H];
        for (nat i = 0; i < context.parameters.H; i++) {
            if (2 + i < count) in[i] = atoll(input[2 + i]);
            else { printf("error: calculate: unexpected end of input\n"); return; }
        }
        const nat result = unreduce(in, context.parameters.m, context.parameters.H);
        printf("%llu\n", result);
        
    } else {
        printf("error: calculate: unknown function: %s\n", input[1]);
        printf("available functions: \n"
               "\t - unreduce <a> <b> <c> <d> <e> <f> <g> <h> ... \n"
               "\t - reduce <value>\n\n");
    }
}

void print_information(char** input, nat count, struct context context) {
    if (count == 1) return;
    else if (strings_equal(input[1], "home")) puts(context.home);
    else if (strings_equal(input[1], "param")) print_parameters(context.parameters);
    
    else if (strings_equal(input[1], "hgrid")) {
        if (is_mn_case(2, 2, context.parameters)) print_m2n2_hgrid(NULL, context.parameters);
        else if (is_mn_case(3, 1, context.parameters)) print_m3n1_hgrid(NULL, context.parameters);
        else print_hgrid(NULL, context.parameters);
        
    } else {
        printf("error: print: unknown info spec: %s\n", input[1]);
        printf("available information: \n"
               "\t param\n"
               "\t hgrid\n"
               "\t home\n\n");
    }
}

int main() {
    struct context context = {default_home};
    
    bool quit = false;
    nat count = 0;
    char buffer[2048] = {0}, *input[2048] = {0};
        
    while (!quit) {
        printf(" 〉");
        fgets(buffer, sizeof buffer, stdin);
        buffer[strlen(buffer) - 1] = '\0';
        split(buffer, input, &count);
        
        if (equals(*input, "quit", "q")) quit = true;
        else if (equals(*input, "clear", "l")) clear_screen();
        else if (equals(*input, "help", "h")) print_help_menu(input, count);
        else if (equals(*input, "print", "p")) print_information(input, count, context);
        else if (equals(*input, "load", "load")) load_file(input, count, &context);
        else if (equals(*input, "calculate", "c")) calculate_function(input, count, context);
        else {
            printf("error: %s: unknown command:\n", buffer);
            print_command(input, count);
        }
    }
}
