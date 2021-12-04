//
//  main.cpp
//  rule110
//
//  Created by Daniel Rehman on 1908294.
//                                                       
//

#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

using nat = size_t;

void print(nat* h, nat l) {
    for (nat i = 0; i < l; i++) {
        if (!h[i]) printf("  ");
        else printf(" O");
    }
    printf("\n");    
}

void randomize(nat* h, nat l) {
    srand((unsigned) time(NULL));
    for (nat i = l; i--;) {
        h[i] = rand() % 2;
    }
}


int main(int argc, const char * argv[]) {

    if (argc < 4) {
        printf("usage: ./program size{nat} delay{us:nat} state{0/1} \n\n");
        exit(1);        
    }
    const nat size = atoi(argv[1]);
    const unsigned delay = atoi(argv[2]);
    const bool should_randomize = atoi(argv[3]);
    
    nat g[size], h[size];
    
    if (should_randomize) {
        randomize(g, size);
    } else {
        memset(g, 0, sizeof g);
        g[0] = 1;
    }
    
    while (true) {
        memcpy(h, g, sizeof h);        
        for (nat j = size; j--;) {
            const nat me = h[j];
            const nat right = h[(j+1) % size];
            const nat left = h[(j + (size - 1)) % size];
            const nat sum = me + right + left;
            const bool special = right and not me and not left;
            g[j] = sum and sum < 3 and not special;
        }
        print(h, size);
        usleep(delay);
    }
}
