//
//  io.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef io_h
#define io_h

#include <stdbool.h>

void clear_screen(void);

char get_character(void);

bool strings_equal(const char* a, const char* b);

bool equals(const char* a, const char* b, const char* abbrev);

#endif /* io_h */
