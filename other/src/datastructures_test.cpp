#include "datastructures.h"

int main(){
    Array<int> arr{10,10};
    arr.fill_random_uniform(0,10);
    arr.print("source array:", " ");
    arr.grow(3,1,0).print("reshaped:" , " ");
}