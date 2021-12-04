#include "utils.h"

#include <cstdlib>
#include <fstream>

float rand_normalized(){
    return (float)(rand()) / (float)(RAND_MAX);
}

constexpr int hash32shift(int key){

    static_assert(sizeof(key) == 4, "hash32shift requires a 32 bit value"); // 32 bits

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

constexpr long long hash64shift(long long key){

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

void viridis(const float value, float& r, float& g, float& b){
    constexpr float c0[3] = {0.2777273272234177, 0.005407344544966578, 0.3340998053353061};
    constexpr float c1[3] = {0.1050930431085774, 1.404613529898575, 1.384590162594685};
    constexpr float c2[3] = {-0.3308618287255563, 0.214847559468213, 0.09509516302823659};
    constexpr float c3[3] = {-4.634230498983486, -5.799100973351585, -19.33244095627987};
    constexpr float c4[3] = {6.228269936347081, 14.17993336680509, 56.69055260068105};
    constexpr float c5[3] = {4.776384997670288, -13.74514537774601, -65.35303263337234};
    constexpr float c6[3] = {-5.435455855934631, 4.645852612178535, 26.3124352495832};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void plasma(const float value, float& r, float& g, float& b){
    constexpr float c0[3] = {0.05873234392399702, 0.02333670892565664, 0.5433401826748754};
    constexpr float c1[3] = {2.176514634195958, 0.2383834171260182, 0.7539604599784036};
    constexpr float c2[3] = {-2.689460476458034, -7.455851135738909, 3.110799939717086};
    constexpr float c3[3] = {6.130348345893603, 42.3461881477227, -28.51885465332158};
    constexpr float c4[3] = {-11.10743619062271, -82.66631109428045, 60.13984767418263};
    constexpr float c5[3] = {10.02306557647065, 71.41361770095349, -54.07218655560067};
    constexpr float c6[3] = {-3.658713842777788, -22.93153465461149, 18.19190778539828};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void magma(const float value, float& r, float& g, float& b){
    constexpr float c0[3] = {-0.002136485053939582, -0.000749655052795221, -0.005386127855323933};
    constexpr float c1[3] = {0.2516605407371642, 0.6775232436837668, 2.494026599312351};
    constexpr float c2[3] = {8.353717279216625, -3.577719514958484, 0.3144679030132573};
    constexpr float c3[3] = {-27.66873308576866, 14.26473078096533, -13.64921318813922};
    constexpr float c4[3] = {52.17613981234068, -27.94360607168351, 12.94416944238394};
    constexpr float c5[3] = {-50.76852536473588, 29.04658282127291, 4.23415299384598};
    constexpr float c6[3] = {18.65570506591883, -11.48977351997711, -5.601961508734096};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void inferno(const float value, float& r, float& g, float& b){
    constexpr float c0[3] = {0.0002189403691192265, 0.001651004631001012, -0.01948089843709184};
    constexpr float c1[3] = {0.1065134194856116, 0.5639564367884091, 3.932712388889277};
    constexpr float c2[3] = {11.60249308247187, -3.972853965665698, -15.9423941062914};
    constexpr float c3[3] = {-41.70399613139459, 17.43639888205313, 44.35414519872813};
    constexpr float c4[3] = {77.162935699427, -33.40235894210092, -81.80730925738993};
    constexpr float c5[3] = {-71.31942824499214, 32.62606426397723, 73.20951985803202};
    constexpr float c6[3] = {25.13112622477341, -12.24266895238567, -23.07032500287172};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}
