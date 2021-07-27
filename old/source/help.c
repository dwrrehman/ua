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
    else if (strings_equal(c, "filter")) printf("\n\tfilter ...::\n\n\t\t a utility for filtering z values, using a list of blacklisted values, or a savelist. only bl is implemented so far.\n");
    else if (strings_equal(c, "search")) printf("\n\t search ::\n\n\t\t search over the unknown values in the current hgrid specified by a file. if thr mode, then record which z values have alifetime length above a given threshold. there will be other modes too.\n");
    else if (strings_equal(c, "visualize")) printf("\n\tvisualize ... ::\n\n\t\t a utility for visualizing cellular automata lifetimes. you can viz a single z value, or an hgrid, or a set of z values from a file.\n");
    else if (strings_equal(c, "generate")) printf("\n\tgenerate ... ::\n\n\t\t a utility for generating ppm images of cellular automata lifetimes, gotten from a file of z values. if begin and end are 0, the whole lifetime is written. its always based on s and t. works best if s^n == t.\n");
    else {
        printf("error: no desciption for command \"%s\"\n", c);
        return;
    }
    print_menu_for(c);
    
    if (strings_equal(c, "visualize")) {
        printf("list of commands for the v set utility:\n"
               
               "    d - scrolls down on the lifetime.\n"
               "    s - scrolls up on the lifetime.\n"
               
               "    f - go to next z value in the z set.\n"
               "    e - go to the previous z value in the z set.\n"
               
               "    w - save the current z value to the savelist file.\n"
               "    a - save the current z value to the blacklist file.\n"
                              
               "    j - move the begin of the visualized slice backwards.\n"
               "    i - move the begin of the visualized slice forwards.\n"
               
               "    o - move the end of the visualized slice backwards.\n"
               "    ; - move the end of the visualized slice forwards.\n"
               
               "    t - toggle whether to display the z value lifetime slice.\n"
               "    p - print the current value of begin_slice and end_slice, and the timestep.\n"
                              
               "    g - break out into command mode, to execute commands in the command line terminal.\n"
               "        use the quit command, as usual, to end up back in the visualizer, right where you left off.\n"
               
               "    Q - end the visualization of the set. you will be prompted to save the save list and black list.\n"
               "    \n"
               );
    }
}



void print_menu_for(const char* command) {
    
    if (strings_equal(command, "load"))
        printf("available file types: \n"
               "\t param(p) <filename> \n"
               "\t hgrid(h) <filename> \n\n");
    
    
    if (strings_equal(command, "calculate"))
        printf("available functions: \n"
               "\t  z\n"
               "\t  hgrid\n"
               "\t  reduce <value>\n"
               "\t  unreduce <a> <b> <c> <d> <e> <f> <g> <h> ... \n"
               "\n");
    
    if (strings_equal(command, "print"))
        printf("available information: \n"
               "\t z\n"
               "\t param(p)\n"
               "\t parameters\n"
               "\t hgrid(h)\n"
               "\t hgrid(h) generic\n"
               "\t vector-hgrid\n"
               "\n");
    
    if (strings_equal(command, "set"))
        printf("available information: \n"
               "\t param(p) <name> <value> \n"
               "\t\t available parameters: \n"
               "\t\t\t nats: m n s t delay(D) initial\n"
               "\t\t\t enum: initial={empty(e), dot(d), random, repeating, centerdot(c)}\n"
               "\t\t\t enum: display={none, numeric(n), intuitive(i), binary(b)}\n"
               "\t\t\t bool: nd={true(1), false(0)}\n"
               "\t z <zvalue> \n"
               "\t hgrid(h) <a> <b> <c> ...\n"
               "\t\t note: \"set hgrid\" simply writes all zeros to a new hgrid.\n"
               "\n");
        
    
    if (strings_equal(command, "visualize"))
        printf("available modes: \n"
               "\t hgrid \n"
               "\t z \n"
               "\t set <begin_index> <begin_slice> <end_slice> <zset_file> <saved_file> <blacklist_file> \n"
               "\n");
    
    
    if (strings_equal(command, "search"))
        printf("available modes: \n"
               "\t threshold <thr> <out_zset_filename> \n"
               "\t target <target:nat> <tolerance:nat> <out_zset_filename> \n"
               "\n");
    
    
    if (strings_equal(command, "filter"))
        printf("available modes: \n"
               "\t blacklist <zset> <blacklist> <outfile> \n"
               "\n");
    
    
    if (strings_equal(command, "generate"))
        printf("available modes: \n"
               "\t lifetimes <zvalues_file> <begin_slice> <end_slice> <destination_dir>\n"
               "\t\t note: use begin=0 and end=0, for the whole lifetime.\n");
    
}


void print_help_menu(char** input, nat count) {
    if (count == 1)
        printf("available commands: \n\n"
               "\t quit(q)\n"
               "\t clear(l)\n"
               "\t help(h)\n"
               "\t print(p)\n"
               "\t load(d)\n"
               "\t calculate(c)\n"
               "\t set(s)\n"
               "\t convert(k)\n"
               "\t filter(f)\n"
               "\t search(t)\n"
               "\t visualize(v)\n"
               "\t generate(g)\n"
               "\n");
    else print_description_for_command(input[1]);
}
