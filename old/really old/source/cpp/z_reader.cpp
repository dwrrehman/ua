//
//  main.cpp
//  z_reader
//
//  Created by Daniel Rehman on 1911203.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <algorithm>

using nat = unsigned long long int;


struct z_pair {
    nat index = 0;
    nat length = 0;
};


void print_vector_of_strings(std::vector<std::string> strings) {
    std::cout << "{ ";
    for (auto string : strings) {
        std::cout << "\"" << string << "\", ";
    }
    std::cout << "}\n";
}

void print_zs(std::vector<z_pair> zs) {
    std::cout << "printing z values : { \n";
    for (auto z : zs) {
        std::cout << z.index << " --> " << z.length << " timesteps." << std::endl;
    }
    std::cout << "}\n";
}

int main() {
    
    std::ifstream file {"/Users/deniylreimn/Documents/wef1.txt"};              //    std::string full_text {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    nat count = 0;
    file >> count;
    
    std::vector<std::string> strings = {};
    std::cout << "reading file...\n";
    while (file.good()) {
        std::string s = {};
        file >> s;
        strings.push_back(s);
    }

    std::vector<z_pair> zs = {};
    std::cout << "parsing file...\n";
    for (nat i = 0; i < strings.size(); i += 36) {
        const nat L = atoll(strings[i + 34].c_str());
        std::string z_string = strings[i + 3];
        z_string.erase(z_string.begin());
        z_string.pop_back();
        z_string.pop_back();
        const nat z = atoll(z_string.c_str());
        zs.push_back({z, L});
    }
    
    printf("found %lu z values...\n", zs.size());
    
    const nat cutoff = 700;
    
    std::cout << "sorting indicies...\n";
    std::sort(zs.begin(), zs.end(), [](z_pair z, z_pair x) { return z.index < x.index; });
    
    std::cout << "pruning uniques...\n";
    zs.erase(std::unique(zs.begin(), zs.end(), [](z_pair z, z_pair x){ return z.index == x.index; }), zs.end());
    
    std::cout << "sorting by lengths...\n";
    std::sort(zs.begin(), zs.end(), [](z_pair z, z_pair x) { return z.length < x.length; });
    
    std::cout << "cutting off lengths less than " << cutoff << "...\n";
    zs.erase(std::remove_if(zs.begin(), zs.end(), [cutoff](z_pair z){ return z.length > cutoff; }), zs.end());
    
    print_zs(zs);
    
    std::ofstream out{"/Users/deniylreimn/Documents/current4.txt", std::ios_base::out | std::ofstream::trunc};
    std::cout << "writing " << zs.size() << " values...\n";
    for (auto z : zs) {
        out << z.index << std::endl;
    }
}
