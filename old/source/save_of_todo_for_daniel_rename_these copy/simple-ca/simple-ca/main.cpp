
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <unistd.h>

#define each(n) (natural i = n.get(); i--;)
#define i(n) (natural i = 0; i < n.get(); i++)
#define j(n) (natural j = 0; j < n.get(); j++)

using natural = size_t;

// ------------ modnat(M) class ---------------------

class mn {
    natural value = 0;
    natural max = 128;
    
public:
    mn(natural v, natural M = 128): max(M), value(v % M) {}
    natural get() {return value;}
    
    mn operator=(mn n) {(value = n.value) %= max; return value;}
    mn operator++(int) {value++; value %= max; return value;}
    
    mn operator+(mn a) {return add(*this, a);}
    mn operator*(mn a) {return multiply(*this, a);}
    mn operator^(mn a) {return exponentiate(*this, a);}
    
    mn add(mn a, mn b) {for each(b) a++; return a;}
    mn multiply(mn a, mn b) {mn c = 0; for each(b) c = c + a; return c;}
    mn exponentiate(mn a, mn b) {mn c = 1; for each(b) c = c * a; return c;}
    
    void print(std::string end = "\n") {std::cout << std::setw(3) << value << end;}    
};


/// for 3d, the indexing equation takes the form:

 //             space[    k * m ^ 0 
//                      + j * m ^ 1 
//                      + ] * m ^ 2      ]


//// and in general:

/*
 mn index(std::vector<mn> space, vector<mn> x, const mn n, const mn m) {
    c = 0;
    for i = 0, i < n, i++ {
        c += x[i] * m ^ i;
    }    
    return space[c];
*/

/// which makes sense. 


////        this is the way we can have a  N dimensional index into a vector representing a N dimensional space.

// ------------ indexers ---------------------

mn index(std::vector<mn> space, std::vector<mn> x, mn n, mn m) {
    mn c = 0;
    for i(n) c = c + (x[i] * (m ^ i));     
    return space[c.get()];
}

mn d2_idx(mn i, mn j, std::vector<mn> space, mn m) { // special case of "index()", where n = 2.
    return space[(j * m + i).get()];
}





/////////////// NEXT STEP:    given this location in the space (via a single index into the vector?) what are its neighors [considering all positions are mod M...]






// ------------ printers ---------------------

void print_space(std::vector<mn> space) { 
    std::cout << space.size() << ": ";
    for (auto unit : space) {
        std::cout << "[";
        unit.print("]");        
    } std::cout << "\n";
}

void print_space_2d(std::vector<mn> space, mn m) {
    std::cout << space.size() << ": ";
    std::cout << "\n";
    for i(m) {
        std::cout << "\t";
        for j(m) {
            std::cout << "[";
            d2_idx(j, i, space, m).print("]"); 
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


int main() {
    {
    int i = 34;
    int m = 4;
    int c = ((i/1 + 1 % m) ) % m;
        
    printf("c = %d\n", c);
    
    exit(0);
    }
    mn n = 2;
    mn m = 5;
    std::vector<mn> space((m ^ n).get(), {0});
    
    mn j = 0;
    for (auto& i : space) {
        i = j++;
    }
    
    std::cout << "raw linear: ";
    print_space(space);
    std::cout << "\n";
    
    std::cout << "2d setup: ";
    print_space_2d(space, m);
    std::cout << "\n";
    
    size_t user_i = 2, user_j = 2;
    while (user_i or user_j) {            // user should say "0 0" to quit.
        std::cout << "give index (x y): ";
        std::cin >> user_i;
        std::cin >> user_j;
        auto res = index(space, {user_j, user_i}, n, m);
        res.print();
        
        sleep(1);        
        print_space_2d(space, m);
    }
    std::cout << "done!\n";
}
