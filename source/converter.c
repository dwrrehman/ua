//
//  binary_expression_converter.c
//
//  Created by Daniel Rehman on 2005085.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 a utility for myself to convert an expression like:
 
            B'E + C'E + CE' + A'B
 
 into a form like:
 
            not B and E or not C and E or C and not E or not A and B
 
 which is needed for the wolfram ANF.
 */
void pretty_print_boolean_expr() {
    char buffer[4096] = {0};
    printf("expression: ");
    fgets(buffer, 4096, stdin);
    for (size_t i = 0; i < strlen(buffer); i++) {
        const char c = buffer[i];
        if (isalpha(c)) {
            if (i + 1 < strlen(buffer)) {
                if (buffer[i + 1] == '\'') { printf("not %c ", c); i++; } else printf("%c ", c);
            } else printf("%c ", c);
            if (i + 1 < strlen(buffer) && isalpha(buffer[i + 1])) printf("and ");
        } else if (c == '+') printf("or ");
    }
    puts("");
}


/**
 a pretty printer for things like:

     (A ∧ B) ⊻ (B ∧ C) ⊻ (B ∧ E) ⊻ (C ∧ E) ⊻ (A ∧ B ∧ C) ⊻ (A ∧ B ∧ E) ⊻ (B ∧ C ∧ E) ⊻ B ⊻ C ⊻ E

 which come from wolfram alpha.
*/
void pretty_print_wolfram_ANF() {
    char buffer[4096] = {0};
    printf("ANF: ");
    fgets(buffer, 4096, stdin);
    printf("\n\n\t\t");
    for (size_t i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == -69) printf(" + ");
        else if (isalpha(buffer[i])) {
            if (buffer[i] == 'A') printf("R");
            else if (buffer[i] == 'B') printf("L");
            else if (buffer[i] == 'C') printf("C");
            else if (buffer[i] == 'D') printf("D");
            else if (buffer[i] == 'E') printf("U");
        }
    } puts("\n\n");
}

void main5(void) {
    pretty_print_boolean_expr();
    pretty_print_wolfram_ANF();
}
