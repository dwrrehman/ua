//
//  io.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef io_h
#define io_h

#include "structures.h"

#include <stdbool.h>

void clear_screen(void);

char get_character(void);

bool strings_equal(const char* a, const char* b);

bool equals(const char* a, const char* b, const char* abbrev);

vector read_nats_from_file(const char* filepath, nat* count);

void write_nats_to_file(const char* filepath, vector v, nat count);

#endif /* io_h */
