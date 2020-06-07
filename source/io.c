//
//  io.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "io.h"

#include "structures.h"
#include "vector.h"


#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void clear_screen() {printf("\e[1;1H\e[2J");}

char get_character() {
    struct termios t = {0}; if (tcgetattr(0, &t) < 0) perror("tcsetattr()");
    t.c_lflag &= ~ICANON; t.c_lflag &= ~ECHO; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t) < 0) perror("tcsetattr ICANON");
    char c = 0; if (read(0, &c, 1) < 0) perror("read()"); t.c_lflag |= ICANON; t.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &t) < 0) perror("tcsetattr ~ICANON");
    return c;
}

bool strings_equal(const char* a, const char* b) {
    return a && b && !strcmp(a, b);
}

bool equals(const char* a, const char* b, const char* abbrev) {
    return strings_equal(a, b) || strings_equal(a, abbrev);
}

nat read_nat(const char* message) {
    char buffer[128] = {0};
    printf("%s", message);
    fgets(buffer, 127, stdin);
    return atoll(buffer);
}

float read_float(const char* message) {
    char buffer[128] = {0};
    printf("%s", message);
    fgets(buffer, 127, stdin);
    return atof(buffer);
}

vector read_nats_from_file(const char* filepath, nat* count) {
        
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }
    
    vector v = NULL;
    char buffer[256] = {0};
    
    while (fgets(buffer, 255, file))
        push_back(&v, count, atoll(buffer));
    
    fclose(file);
    
    return v;
}

void write_nats_to_file(const char* filepath, vector v, nat count) {
    
    FILE* file = fopen(filepath, "w+");
    if (!file) {
        perror("fopen");
        return;
    }
    
    for (nat i = 0; i < count; i++)
        fprintf(file, "%llu\n", v[i]);
    
    fclose(file);
}

