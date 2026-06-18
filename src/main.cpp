#include <graphics_test_chronograph.h>
#include <graphics_test_helmet.h>
#include <graphics_test_skull.h>
#include <ngrid_test.h>
#include <particles_test.h>
#include <iostream>

int main() {
	//Log::set_level(LogLevel::LEVEL_FORCE);
	//Log::enable_exit_on_warning();
	//ngrid_test();
	//graphics_test_skull();
	//graphics_test_helmet();
	//graphics_test_chronograph();
	particles_test();

	std::cin.get();
}

