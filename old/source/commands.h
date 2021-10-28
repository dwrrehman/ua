//
//  commands.h
//  ua
//
//  Created by Daniel Rehman on 2006044.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef commands_h
#define commands_h

#include "structures.h"

void print_command(char** commands, nat count);

void split(char* input, char** command, nat* count);

#endif /* commands_h */