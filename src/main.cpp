#include <log.h>
#include <ngrid_test.h>
#include <vkcontext.h>
#include <vkcontext_graphics_test.h>

int main() {
	Log::set_level(LogLevel::LEVEL_WARNING);
	//ngrid_test();
	vkcontext_graphics_test();

	std::cin.get();
}

