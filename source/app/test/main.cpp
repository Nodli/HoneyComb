#include "GL.h"
#include "OpenGLWindow.h"

#include "MemoryPool.h"

#include "print.h"

void memory_allocator(){
    MemoryPool<sizeof(int)> pool;
    pool.allocate(2);

    int* A = pool.get<int>();
    int* B = pool.get<int>();

    *A = 1;
    *B = 2;

    pool.give(A);
    pool.give(B);

    int* C = pool.get<int>();
    int* D = pool.get<int>();

    print(A, D, A == D);
    print(B, C, B == C);

    int E = 0;
    int* F = &E;
    //pool.give(F);
}

int main(){

    memory_allocator();

}
