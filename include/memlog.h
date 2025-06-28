// author: cyberchriz (Christian Suer)

// this code logs any heap memory allocations to the console
// by overriding the `new` and 'delete' operators;
// in order to use this, simply use a "#define MEMLOG" flag as a preprocessor directive before(!) including this file

#ifndef MEMLOG_H // include guard
#define MEMLOG_H




#ifdef MEMLOG

// dependencies
#include <cstdlib>
#include <iostream>
#include <unordered_map>

// global variables

std::unordered_map<void*, std::size_t> allocated_memory;
int total_allocation = 0;

    // operator 'new' override
void* operator new(std::size_t size) {
    void* ptr = std::malloc(size);
    allocated_memory.insert(ptr, size);
    total_allocation += size;
    std::cout << " allocated " << size << " bytes at address " << ptr;
    std::cout << " [total: " << total_allocation << " bytes]" << std::endl;
    return ptr;
}

// operator 'delete' override
void operator delete(void* ptr) noexcept {
    std::size_t size = allocated_memory[ptr];
    total_allocation -= size;
    std::cout << " freed " << size << " bytes at address " << ptr;
    std::cout << " [total: " << total_allocation << " bytes]" << std::endl;
    allocated_memory.erase(ptr);
    std::free(ptr);
}

#endif






#endif // end of include guard