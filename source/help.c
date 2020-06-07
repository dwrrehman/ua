//
//  help.c
//  ua
//
//  Created by Daniel Rehman on 2006044.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "help.h"

#include "io.h"

void print_description_for_command(const char* c) {
    if (0) {}
    
    else if (strings_equal(c, "quit")) printf("\n\tquit ::\n\n\t\t quit the UA terminal utility.\n");
    else if (strings_equal(c, "clear")) printf("\n\tclear ::\n\n\t\t clear the screen.\n");
    else if (strings_equal(c, "help")) printf("\n\thelp [command]::\n\n\t\t print the help menu, or descriptions for commands, if one is given.\n");
    else if (strings_equal(c, "print")) printf("\n\tprint ... ::\n\n\t\t display helpful information about the internal state of the system. this includes parameter information, h grids, z values, etc.\n");
    else if (strings_equal(c, "load")) printf("\n\tload ... ::\n\n\t\t load information from a file. either a parameter file or a hgrid file can be specified. \n");
    else if (strings_equal(c, "calculate")) printf("\n\tcalculate ... ::\n\n\t\t calculate a mathematical CA-useful function, such as a reduce or unreduce.\n");
    else if (strings_equal(c, "set")) printf("\n\tset ... ::\n\n\t\t set parameter values, or the h grid or z values by hand, as opposed to loading them from a file.\n");
    else if (strings_equal(c, "convert")) printf("\n\tconvert ::\n\n\t\t a quick and dirty utility used for translating boolean expressions into ANF, and into a prettier representation. not for general use, only useful for 2,2, really.\n");
    else if (strings_equal(c, "search")) printf("\n\t search ::\n\n\t\t search over the unknown values in the current hgrid specified by a file. if thr mode, then record which z values have alifetime length above a given threshold. there will be other modes too.\n");
    else if (strings_equal(c, "visualize")) printf("\n\tvisualize ... ::\n\n\t\t a utility for visualizing cellular automata lifetimes. you can viz a single z value, or an hgrid, or a set of z values from a file.\n");
    else {
        printf("error: no desciption for command \"%s\"\n", c);
        return;
    }
    printf("\n\t note: type the command for more info on \"...\") \n\n");
}

void print_help_menu(char** input, nat count) {
    if (count == 1)
        printf("available commands: \n\n"
               "\t quit(q)\n"
               "\t clear(l)\n"
               "\t help\n"
               "\t print(p)\n"
               "\t load\n"
               "\t calculate(c)\n"
               "\t set\n"
               "\t convert\n"
               "\t search(s)\n"
               "\t visualize(v)\n"
               "\n");
    else print_description_for_command(input[1]);
}
