//
//  vector.c
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

vector create(nat count) {
    return calloc(count, sizeof(element));
}

void destroy(vector* v) {
    free(*v);
    *v = NULL;
}

vector duplicate(vector v, nat count) {
    vector c = create(count);
    memcpy(c, v, sizeof(element) * count);
    return c;
}

void set(nat value, vector v, nat count) {
    memset(v, value, sizeof(element) * count);
}

void push_back(vector* v, nat* count, element e) {
    *v = realloc(*v, sizeof(element) * (*count + 1));
    (*v)[(*count)++] = e;
}

void pop_back(vector* v, nat* count) {
    if (!*count) return;
    (*count)--;
    *v = realloc(*v, sizeof(element) * (*count));
}

element pop(vector* v, nat* count) {
    if (!*count) abort();
    (*count)--;
    element e = (*v)[*count];
    *v = realloc(*v, sizeof(element) * (*count));
    return e;
}

element back(vector v, nat count) {
    if (!count) abort();
    return v[count - 1];
}

nat sum(vector v, nat count) {
    nat s = 0;
    
    for (nat i = 0; i < count; i++)
        s += v[i];
    
    return s;
}

bool vectors_equal
(vector a, nat a_count,
 vector b, nat b_count) {
    
    if (a_count != b_count)
        return false;
    
    for (nat i = 0; i < a_count; i++)
        if (a[i] != b[i])
            return false;
    
    return true;
}

nat unreduce(vector v, nat radix, nat length) {
    nat s = 0, p = 1;
    for (nat i = 0; i < length; i++, p *= radix)
        s += p * v[i];
    return s;
}

void reduce(vector out, nat s, nat radix, nat length) {
    for (nat i = 0, p = 1; i < length; i++, p *= radix)
        out[i] = (s / p) % radix;
}

bool contains(vector v, nat count, element e) {
    for (nat i = 0; i < count; i++)
        if (v[i] == e)
            return true;
    return false;
}


void print_vector(vector v, nat count) {
    printf("{ ");
    for (nat i = 0; i < count; i++) printf("%llu ", v[i]);
    printf("}");
}

void print_vector_line(vector v, nat count) {
    printf("{ ");
    for (nat i = 0; i < count; i++) printf("%llu ", v[i]);
    printf("}\n");
}

void print_vector_line_message(const char* message, vector v, nat count) {
    printf("%s : ", message);
    print_vector_line(v, count);
}
