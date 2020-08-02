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
#include "converter.h"

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

const char* default_home = "/Users/deniylreimn/Documents/projects/ua/";

void load_file(char** input, nat count, struct context* context) {
    
    const nat n = context->parameters.n;
    
    if (equals(input[1], "param", "p")) load_parameters_from_file(input[2], context);
    else if (equals(input[1], "hgrid", "h")) {
        if (n == 3) load_n3_hgrid(input[2], context);
        if (n == 4) load_n4_hgrid(input[2], context);
        if (n == 5) load_n5_hgrid(input[2], context);
    } else {
        printf("error: load: unknown load type: %s\n", input[1]);
        print_menu_for("load");
    }
}

void read_values_from_input(vector out, nat in_length, char** input, nat input_count, nat offset) {
    for (nat i = 0; i < in_length; i++) {
        if (i + offset < input_count) {
            out[i] = atoll(input[i + offset]);
        } else {
            printf("error: unexpected end of input\n");
            return;
        }
    }
}

void calculate_function(char** input, nat count, struct context context) {
    
    const nat m = context.parameters.m, n = context.parameters.n, H = to(m,n);
    
    if (strings_equal(input[1], "z"))
        printf("%llu\n", unreduce(context.hgrid, m, H));
    
    else if (strings_equal(input[1], "hgrid")) {
        reduce(context.hgrid, context.z, m, H);
        print_vector_line_message("hgrid = ", context.hgrid, H);
    
    } else if (strings_equal(input[1], "reduce")) {
        element out[H];
        reduce(out, atoll(input[2]), m, H);
        print_vector_line(out, H);
        
    } else if (strings_equal(input[1], "unreduce")) {
        element in[H]; fill(0, in, H);
        read_values_from_input(in, H, input, count, 2);
        printf("%llu\n", unreduce(in, m, H));
        
    } else {
        printf("error: calculate: unknown function: %s\n", input[1]);
        print_menu_for("calculate");
    }
}

void print_information(char** input, nat count, struct context context) {
    
    const nat m = context.parameters.m, n = context.parameters.n, H = to(m,n);
    
    if (strings_equal(input[1], "home")) puts(context.home);
    else if (strings_equal(input[1], "z")) printf("z = %llu\n", context.z);
    else if (equals(input[1], "param", "p")) print_parameters(context.parameters);
    else if (strings_equal(input[1], "parameters")) verbose_print_parameters(context.parameters);
    else if (strings_equal(input[1], "vector-hgrid")) print_vector_line_message("hgrid = ", context.hgrid, H);
        
    else if (equals(input[1], "hgrid", "h"))
        print_hgrid(context.hgrid, context.parameters, strings_equal(input[2], "generic"));
    else {
        printf("error: print: unknown info spec: %s\n", input[1]);
        print_menu_for("print");
    }
}

void set(char** input, nat count, struct context* context) {
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    if (strings_equal(input[1], "param")) set_parameter(&context->parameters, input[2], input[3]);
    else if (strings_equal(input[1], "z") && input[2]) context->z = atoll(input[2]);
    else if (strings_equal(input[1], "hgrid")) {
        destroy(&context->hgrid);
        context->hgrid = create(H);
        read_values_from_input(context->hgrid, H, input, count, 2);
    } else {
        printf("error: set: unknown target: %s\n", input[1]);
        print_menu_for("set");
    }
}

void visualize(char** input, nat count, struct context* context) {
    
    const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
    if (strings_equal(input[1], "hgrid"))
        visualize_lifetime(0, 0, 0, context->hgrid, context->parameters);
    
    else if (strings_equal(input[1], "z")) {
        element hgrid[H];
        reduce(hgrid, context->z, m, H);
        visualize_lifetime(0, 0, 0, hgrid, context->parameters);
                    
    } else if (strings_equal(input[1], "set")) {
        
        if (count != 8) {
            printf("error: visualize set: incorrect number of arguments!\n");
            print_menu_for("visualize");
            return;
        }
        
        nat z_count = 0;
        vector zset = read_nats_from_file(input[5], &z_count);
        visualize_set(atoll(input[2]), atoll(input[3]), atoll(input[4]), zset, z_count, input[6], input[7], context);
       
    } else {
        printf("error: visualize: unknown mode: %s\n", input[1]);
        print_menu_for("visualize");
    }
}

void search(char** input, nat input_count, struct context* c) {
    
    if (strings_equal(input[1], "threshold")) {
        threshold_search(atoll(input[2]), input[3], c);
        
    } else if (strings_equal(input[1], "other")) {
        printf("unimplemented.\n");
                
    } else {
        printf("error: search: unknown search type: %s\n", input[1]);
        print_menu_for("search");
    }
}

void filter_utility(char** input, nat input_count) {
    if (strings_equal(input[1], "blacklist"))
        filter_using_blacklist(input);
    else {
        printf("error: filter: unknown filter mode: %s\n", input[1]);
        print_menu_for("filter");
    }
}

void generate_utility(char** input, nat input_count, struct context* context) {
    if (strings_equal(input[1], "lifetimes"))
        generate_lifetime_images(input, context);
    else {
        printf("error: generate: unknown generation mode: %s\n", input[1]);
        print_menu_for("generate");
    }
}

void user_interface(struct context *context) {
    bool quit = false;
    while (!quit) {
        nat count = 0;
        char* input[2048] = {0};
        char* line = readline(" 〉");
        add_history(line);
        split(line, input, &count);
        
        if (equals(*input, "", "")) {}
        else if (equals(*input, "quit", "q")) quit = true;
        else if (equals(*input, "clear", "l")) clear_screen();
        else if (equals(*input, "help", "h")) print_help_menu(input, count);
        else if (equals(*input, "print", "p")) print_information(input, count, *context);
        else if (equals(*input, "load", "d")) load_file(input, count, context);
        else if (equals(*input, "calculate", "c")) calculate_function(input, count, *context);
        else if (equals(*input, "set", "s")) set(input, count, context);
        else if (equals(*input, "convert", "k")) convert_expressions();
        else if (equals(*input, "filter", "f")) filter_utility(input, count);
        else if (equals(*input, "search", "t")) search(input, count, context);
        else if (equals(*input, "visualize", "v")) visualize(input, count, context);
        else if (equals(*input, "generate", "g")) generate_utility(input, count, context);
        else {
            printf("error: %s: unknown command:\n", line);
            print_command(input, count);
        }
    }
}

int main() {
    struct context context = {
        .home = default_home,
        .parameters = {0},
        .hgrid = NULL,
        .z = 0
    };
    using_history();
    signal(SIGINT, handler);
    user_interface(&context);
    clear_history();
}
