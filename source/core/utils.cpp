#include "utils.h"

#include <cstdlib>
#include <fstream>

float rand_normalized(){
    return (float)(rand()) / (float)(RAND_MAX);
}

constexpr int hash32shift(int key){

    static_assert(sizeof(key) == 4, "hash32shift requieres a 32 bit value"); // 32 bits

    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
}

constexpr int hash32shiftmult(int key){

    static_assert(sizeof(key) == 4, "hash32shiftmult requieres a 32 bit value"); // 32 bits

    key = (key ^ 61) ^ (key >> 16);
    key *= 9;
    key = key ^ (key >> 4);
    key *= 0x27d4eb2d;
	key = key ^ (key >> 15);
	return key;
}

constexpr long hash64shift(long key){

    static_assert(sizeof(key) == 8, "hash64shift requieres a 64 bit value"); // 32 bits

    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

void read_file(const char* const file_path, std::string& output){
    std::ifstream file(file_path, std::ios::in);

    if(file.is_open()){
        file.seekg(0, std::ios::end);
        output.resize(file.tellg());

        file.seekg(0, std::ios::beg);
        file.read(&output.front(), output.size());
    }

    file.close();
}

std::string read_file(const char* const file_path){
    std::string output;
    read_file(file_path, output);

    return output;
}
