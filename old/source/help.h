//
//  help.h
//  ua
//
//  Created by Daniel Rehman on 2006044.
//                                                       
//

#ifndef help_h
#define help_h

#include "structures.h"

#include <stdio.h>

void print_description_for_command(const char* c);

void print_help_menu(char** input, nat count);

void print_menu_for(const char* command);


#endif /* help_h */
