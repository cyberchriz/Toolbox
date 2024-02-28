#ifndef SEED_H
#define SEED_H

#include <chrono>

// Function to extract a 32-bit seed from std::chrono::time_point
uint32_t seed32() {
	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
	std::chrono::milliseconds ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	return static_cast<uint32_t>(ms_since_epoch.count());
}

#endif