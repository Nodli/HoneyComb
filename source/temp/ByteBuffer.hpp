#ifndef VP_BYTEBUFFER_HPP
#define VP_BYTEBUFFER_HPP

#include <fstream>
#include <vector>
#include <cstdint>

struct ByteBuffer
{
    template<typename T>
    void push(const T val);

    std::vector<char> mContent;
};

template <typename T> void putBytes(const T val, std::ofstream& stream);
template <typename T> T getBytes(std::ifstream& stream);

// template definition
template<typename T>
void ByteBuffer::push(const T val)
{
    const char* val_bytes = (const char*)(&val);

    for(unsigned int ichar = 0; ichar != sizeof(T); ++ichar){
        mContent.push_back(val_bytes[ichar]);
    }
}

template<typename T>
void putBytes(const T val, std::ofstream& stream){
    const char* val_bytes = (const char*)(&val);

    stream.write(val_bytes, sizeof(T));
}

template <typename T> T getBytes(std::ifstream& stream)
{
    T     val       = 0;
    char* val_bytes = (char*)(&val);
    stream.read(val_bytes, sizeof(T));

    return val;
}

#endif
