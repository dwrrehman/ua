//
//  vector.h
//  ua
//
//  Created by Daniel Rehman on 2006022.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#ifndef vector_h
#define vector_h

#include "structures.h"

/**
 create a new vector of nats.
 */
vector create(nat count);

/**
 destroy an already created vector.
*/
void destroy(vector* v);

/**
 duplicate an existing vector of nats.
*/
vector duplicate(vector v, nat count);

/**
 fill the given vector entirely
 with one particular value.
*/
void fill(nat value, vector v, nat count);

/**
 push a nat onto the back of a vector.
*/
void push_back(vector* v, nat* count, element e);

/**
 remove the last element in a vector.
 must be non empty.
 doesnt return that element. use pop() if you want it returned.
*/
void pop_back(vector* v, nat* count);

/**
 remove the last element in a vector.
 must be non empty.
 returns the element that was popped.
*/
element pop(vector* v, nat* count);

/**
 return the element at the end of the vector.
*/
element back(vector v, nat count);

/**
 compute the sum of the vector.
*/
nat sum(vector v, nat count);

/**
 tell if the vectors are equal.
*/

bool vectors_equal(vector a, nat al, vector b, nat bl);

/**
 treat the vector as a radix-ary
 representation of (with length number
 of "bits"(but radixary).) as a number, and return the
 number the vector represents.
 
 eg,
 
 with radix = 2, length = 3, vector in = [1,1,1]
 the output would be 7.
 and similarly,
 
 with radix = 4, length = 2, vector in = [3,2]
 the outptu would be 3 + 8 = 11. the output would be 11.
*/
nat unreduce(vector in, nat radix, nat length);

/**
 convert the given natural number,
 s, into a radix-ary string of nats,
 with given radix, with a total of length "bits" (but radixary).
 
 fills the vector with these radix-ary nats.
 
 eg,
 
 with radix = 2, length = 3, s = 7
 the output vector would be [1,1,1].
 and similarly,
 
 with radix = 4, length = 2, s = 11,
 the outptu would be the vector [3,2].
*/
void reduce(vector out, nat s, nat radix, nat length);

/**
 tell if a vector contains a given nat.
 */
bool contains(vector v, nat count, element e);

/**
 print the vector simply. NO newline.
*/
void print_vector(vector v, nat count);

/**
 print the vector simply. appends a newine at the end.
*/
void print_vector_line(vector v, nat count);

/**
 print the vector, but with a message preceeding it, and a newline at the end.
 
 eg:
    my message here: { 1 2 3 4 5 }\n
    
*/
void print_vector_line_message(const char* message, vector v, nat count);

#endif /* vector_h */
