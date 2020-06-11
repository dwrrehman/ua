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
    if (strings_equal(input[1], "param")) load_parameters_from_file(input[2], context);
    else if (strings_equal(input[1], "hgrid")) {
        if (strings_equal(input[2],"2,2")) load_m2n2_hgrid(input[3], context);
        else if (strings_equal(input[2],"3,1")) load_m3n1_hgrid(input[3], context);
        else if (strings_equal(input[2],"generic")) load_hgrid(input[3], context);
    } else {
        printf("error: load: unknown load type: %s\n", input[1]);
        printf("available file types: \n"
               "\t param <filename> \n"
               "\t hgrid <2,2/3,1/generic> <filename> \n\n");
    }
}

void read_values_from_input(vector in, nat in_length, char** input, nat input_count, nat offset) {
    for (nat i = 0; i < in_length; i++) {
        if (i + offset < input_count) {
            in[i] = atoll(input[i + offset]);
        } else {
            printf("error: unexpected end of input\n");
            return;
        }
    }
}

void calculate_function(char** input, nat count, struct context context) {
    const nat H = context.parameters.H, m = context.parameters.m;
    
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
        printf("available functions: \n"
               "\t  z\n"
               "\t  hgrid\n"
               "\t  reduce <value>\n"
               "\t  unreduce <a> <b> <c> <d> <e> <f> <g> <h> ... \n"
               "\n");
    }
}

void print_information(char** input, nat count, struct context context) {
    if (strings_equal(input[1], "home")) puts(context.home);
    else if (strings_equal(input[1], "z")) printf("z = %llu\n", context.z);
    else if (strings_equal(input[1], "param")) print_parameters(context.parameters);
    else if (strings_equal(input[1], "parameters")) verbose_print_parameters(context.parameters);
    else if (strings_equal(input[1], "vector-hgrid")) print_vector_line_message("hgrid = ", context.hgrid, context.parameters.H);
    else if (strings_equal(input[1], "hgrid")) {
        ///TODO: make these print functions generic over m. havbe a 1d and a 2d printing functions.
        if (is_mn_case(2, 2, context.parameters)) print_m2n2_hgrid(context.hgrid, context.parameters);
        else if (is_mn_case(3, 1, context.parameters)) print_m3n1_hgrid(context.hgrid, context.parameters);
        else print_hgrid(context.hgrid, context.parameters);
    } else {
        printf("error: print: unknown info spec: %s\n", input[1]);
        printf("available information: \n"
               "\t home\n"
               "\t z\n"
               "\t param\n"
               "\t parameters\n"
               "\t hgrid\n"
               "\t vector-hgrid\n"
               "\n");
    }
}

void set(char** input, nat count, struct context* context) {
    if (strings_equal(input[1], "param")) set_parameter(&context->parameters, input[2], input[3]);
    else if (strings_equal(input[1], "z")) context->z = atoll(input[2]);
    else if (strings_equal(input[1], "hgrid")) {
        destroy(&context->hgrid);
        context->hgrid = create(context->parameters.H);
        read_values_from_input(context->hgrid, context->parameters.H, input, count, 2);
    } else {
        printf("error: set: unknown target: %s\n", input[1]);
        printf("available information: \n"
               "\t param <name> <value>\n"
               "\t z <zvalue> ...\n"
               "\t hgrid <a> <b> <c> ...\n"
               "\n");
    }
}

void visualize(char** input, nat count, struct context* context) {
    
    if (strings_equal(input[1], "hgrid"))
        visualize_lifetime(0, 0, 0, context->hgrid, context->parameters);
    
    else if (strings_equal(input[1], "z")) {
        element hgrid[context->parameters.H];
        reduce(hgrid, context->z, context->parameters.m, context->parameters.H);
        visualize_lifetime(0, 0, 0, hgrid, context->parameters);
                    
    } else if (strings_equal(input[1], "set")) {
                        
        if (count != 6) {
            printf("error: visualize set: incorrect number of arguments! expected: \n"
                   "\t set <begin_index> <zset_file> <saved_file> <blacklist_file> \n");
            return;
        }
        
        nat z_count = 0;
        vector zset = read_nats_from_file(input[3], &z_count);
        visualize_set(atoll(input[2]), zset, z_count, input[4], input[5], context);
       
    } else {
        printf("error: visualize: unknown mode: %s\n", input[1]);
        printf("available modes: \n"
               "\t hgrid \n"
               "\t z \n"
               "\t set <begin_index> <zset_file> <saved_file> <blacklist_file> \n"
               "\n");
    }
}

void search(char** input, nat input_count, struct context* c) {
    
    if (strings_equal(input[1], "threshold")) {
        threshold_search(atoll(input[2]), input[3], c);
        
    } else if (strings_equal(input[1], "other")) {
        printf("unimplemented.\n");
                
    } else {
        printf("error: search: unknown search type: %s\n", input[1]);
        printf("available modes: \n"
               "\t threshold <thr> <out_zset_filename> \n"
               "\n");
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
        else if (equals(*input, "help", "help")) print_help_menu(input, count);
        else if (equals(*input, "print", "p")) print_information(input, count, *context);
        else if (equals(*input, "load", "load")) load_file(input, count, context);
        else if (equals(*input, "calculate", "c")) calculate_function(input, count, *context);
        else if (equals(*input, "set", "set")) set(input, count, context);
        else if (equals(*input, "convert", "convert")) convert_expressions();
        else if (equals(*input, "search", "s")) search(input, count, context);
        else if (equals(*input, "visualize", "v")) visualize(input, count, context);
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
}
