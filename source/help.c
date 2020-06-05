//
//  help.c
//  ua
//
//  Created by Daniel Rehman on 2006044.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "help.h"

void print_description_for_command(const char* c) {
    printf("no descriptions for commands have been added yet.\n");
}

void print_help_menu(char** input, nat count) {
    if (count == 1)
        printf("available utilities: \n\n"
               "\t quit\n"
               "\t help\n"
               "\t print\n"
               "\t load\n"
               "\t calculate\n"
               "\n");
    else print_description_for_command(input[2]);
}
