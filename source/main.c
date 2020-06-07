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

const char* default_home = "/Users/deniylreimn/Documents/projects/ua/testing/";

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
            in[i] = atoll(input[i + offset ]);
        } else {
            printf("error: unexpected end of input\n");
            return;
        }
    }
}

void calculate_function(char** input, nat count, struct context context) {
    const nat H = context.parameters.H;
    if (strings_equal(input[1], "z"))
        printf("%llu\n", unreduce(context.hgrid, context.parameters.m, H));
    
    else if (strings_equal(input[1], "reduce")) {
        element out[H];
        reduce(out, atoll(input[2]), context.parameters.m, H);
        print_vector_line_message("result = ", out, H);
        
    } else if (strings_equal(input[1], "unreduce")) {
        element in[H]; fill(0, in, H);
        read_values_from_input(in, H, input, count, 2);
        printf("%llu\n", unreduce(in, context.parameters.m, H));
    } else {
        printf("error: calculate: unknown function: %s\n", input[1]);
        printf("available functions: \n"
               "\t  z\n"
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
    else if (strings_equal(input[1], "hgrid")) {
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

nat find_unknowns(vector hg, vector I, nat H) {
    nat count = 0;
    for (nat i = 0; i < H; i++)
        if (hg[i] == unknown_dummy_value) I[count++] = i;
    return count;
}

struct score {
    nat score;
    nat z;
};

void lifetime_threshold_search(char** input, nat count, struct context* c) {
    
    const nat threshold = atoll(input[1]);
    const nat
        m = c->parameters.m,
        H = c->parameters.H;
    
    vector hg = duplicate(c->hgrid, H);
    element indicies[H], search[H];
    
    const nat
        u = find_unknowns(hg, indicies, H),
        Z = powl(m, u);
    
    printf("searching over %llu unknowns...\n", u);

    element definitions[Z], scores[Z];
    nat z_value_count = 0;
    
    for (nat z = 0; z < Z; z++) {
        printf("\r [  %llu  /  %llu  ]       ", z, Z);
        reduce(search, z, m, u);
        map(hg, search, indicies, H);
        const nat score = measure_lifetime_for_hgrid(hg, &c->parameters);
        const nat definition = unreduce(hg, m, H);
        
        if (score >= threshold) {
             printf("\n[z = %llu] ---> %llu timesteps\n\n",
                    definition, score);
            
            definitions[z_value_count] = definition;
            scores[z_value_count] = score;
            z_value_count++;
        }
    }
    puts("\n");
    printf("found %llu z values above threshold. (%f%%) \n", z_value_count, ((float)z_value_count / Z));
}

int main(void) {
    struct context context = {default_home};
    bool quit = false;
    nat count = 0;
    char* input[2048] = {0}, *line = 0;
    using_history();
    while (!quit) {
        line = readline(" 〉");
        add_history(line);
        split(line, input, &count);
        
        if (equals(*input, "", "")) {}
        else if (equals(*input, "quit", "q")) quit = true;
        else if (equals(*input, "clear", "l")) clear_screen();
        else if (equals(*input, "help", "h")) print_help_menu(input, count);
        else if (equals(*input, "print", "p")) print_information(input, count, context);
        else if (equals(*input, "load", "load")) load_file(input, count, &context);
        else if (equals(*input, "calculate", "c")) calculate_function(input, count, context);
        else if (equals(*input, "set", "set")) set(input, count, &context);
        else if (equals(*input, "threshold", "thr")) lifetime_threshold_search(input, count, &context);
        else if (equals(*input, "convert", "v")) convert_expressions();
        else {
            printf("error: %s: unknown command:\n", line);
            print_command(input, count);
        }
    }
}
