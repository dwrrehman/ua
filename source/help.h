//
//  help.h
//  ua
//
//  Created by Daniel Rehman on 2006044.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef help_h
#define help_h

#include "structures.h"

#include <stdio.h>

void print_description_for_command(const char* c);
void print_help_menu(char** input, nat count);

#endif /* help_h */