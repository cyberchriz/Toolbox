#include "../headers/vkvec.h"

int main() {
	Log::set_level(INFO);
	VkVec A(100000);
	A.fill_random_uniform(0,1);
	std::cout << "result = " << A.min() << std::endl;
} 
