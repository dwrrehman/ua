//
//  commands.c
//  ua
//
//  Created by Daniel Rehman on 2006044.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "commands.h"
#include "structures.h"
#include "parameters.h"
#include "hgrid.h"

#include <stdio.h>
#include <string.h>


void print_command(char** commands, nat count) {
    printf("command: (%llu) {\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("\targ #%lu: \"%s\"\n", i, commands[i]);
    }
    printf("}\n");
}

void split(char* input, char** command, nat* count) {
    char* current = input, *end = input;
    *count = 0;
    while (current) {
        strsep(&end, " ");
        command[(*count)++] = current;
        current = end;
    }
}
