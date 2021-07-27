/// current state of the ua theory.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned long long nat;

int main(int argc, const char** argv) {
    
    if (argc <= 7) exit(puts("new: 24: usage: \n\t./ca m n s l t D d\n"));
                        
    const nat
        modulus = atoll(argv[1]),
        numerus = atoll(argv[2]),
        cell_count = atoll(argv[3]),
        sidelength = atoll(argv[4]),
        timesteps = atoll(argv[5]),
        delay = atoll(argv[6]),
        dimensional = atoll(argv[7]);
    
    nat write_cells[cell_count], read_cells[cell_count], neighborhood[numerus];
    
    memset(write_cells, 0, sizeof write_cells);
    ++*write_cells;
    
    for (nat timestep = 0; timestep < timesteps; timestep++) {
        
        if (dimensional) printf("\e[1;1H\e[2J");
        memcpy(read_cells, write_cells, sizeof read_cells);
        
        for (nat cell = 0; cell < cell_count; cell++) {
            
            nat y = 0;
            if (y < numerus) neighborhood[y++] = read_cells[cell];
            for (nat x = 1; x < cell_count; x *= sidelength) {
                if (y < numerus) neighborhood[y++] = read_cells[cell + x * ((cell / x + sidelength + 1) % sidelength - cell / x % sidelength)];
                if (y < numerus) neighborhood[y++] = read_cells[cell + x * ((cell / x + sidelength - 1) % sidelength - cell / x % sidelength)];
            }
            
//            for (nat i = 1; i < numerus - 1; i++) {
//                write_cells[cell] += neighborhood[numerus - 1] * (neighborhood[i] + 1);
//            }
            
            nat* const h = neighborhood;
            
            if (numerus == 5) {
                write_cells[cell] += h[4] * (h[3] * (h[2] * (h[1] * h[0] + 1) + 1) + 1);
            }
    
            
            write_cells[cell] %= modulus;
            if (cell % sidelength == 0 && dimensional) printf("\n");
            printf("\033[38;5;%um██\033[0m", (unsigned)((double) read_cells[cell] / modulus * 24) + 232);
        }
        printf("\n");
        fflush(stdout);
        if (!delay) getchar(); else usleep((unsigned) delay);
    }
}
