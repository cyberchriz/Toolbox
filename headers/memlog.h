// author: cyberchriz (Christian Suer)

// this code logs any heap memory allocations to the console
// by overriding the `new` and 'delete' operators;
// in order to use this, simply use a "#define MEMLOG" flag as a preprocessor directive before(!) including this file

#ifndef MEMLOG_H
#define MEMLOG_H

#ifdef MEMLOG

    // dependencies
    #include <cstdlib>
    #include <cxxabi.h>
    #include <execinfo.h>
    #include <iostream>
    #include <unordered_map>

    // global variables
    
    std::unordered_map<void*, std::size_t> allocated_memory;
    int total_allocation=0;

    // get function name from callstack
    // level 0: current function
    // level 1: calling function
    void get_caller_function_name(char*& func_name, int stack_level=1){
        void* callstack[stack_level+1];
        int num_frames = backtrace(callstack, stack_level+1);
        if (num_frames >= 2) {
            char** symbollist = backtrace_symbols(callstack, stack_level+1);
            if (symbollist != nullptr) {
                int status = 0;
                func_name = abi::__cxa_demangle(symbollist[stack_level], nullptr, nullptr, &status);
                free(symbollist);
            }
        }
    }

    // operator 'new' override
    void* operator new(std::size_t size){
        void* ptr = std::malloc(size);
        allocated_memory[ptr] = size;
        total_allocation+=size;

        char* func_name = nullptr;
        get_caller_function_name(func_name, 1);
        if (func_name != nullptr) {
            std::cout << "In Function " << func_name;            
        }
        get_caller_function_name(func_name, 2);
        if (func_name != nullptr) {
            std::cout << " (called by function " << func_name << ")";
        }
        std::cout << " allocated " << size << " bytes at address " << ptr;
        std::cout << " [total: " << total_allocation << " bytes]" << std::endl;
        free(func_name);
        return ptr;
    }

    // operator 'delete' override
    void operator delete(void* ptr) noexcept {
        std::size_t size = allocated_memory[ptr];
        total_allocation-=size;

        char* func_name = nullptr;
        get_caller_function_name(func_name, 1);
        if (func_name != nullptr) {
            std::cout << "In Function " << func_name;            
        }
        get_caller_function_name(func_name, 2);
        if (func_name != nullptr) {
            std::cout << " (called by function " << func_name << ")";
        }
        std::cout << " freed " << size << " bytes at address " << ptr;
        std::cout << " [total: " << total_allocation << " bytes]" << std::endl;
        free(func_name);
        allocated_memory.erase(ptr);
        std::free(ptr);
    }

#endif

#endif