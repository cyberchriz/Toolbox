#include <log.h>
#include <ngrid_test.h>
#include <vkcontext.h>
#include <vkcontext_graphics_test_helmet.h>
#include <vkcontext_graphics_test_skull.h>

int main() {
	//Log::set_level(LogLevel::LEVEL_FORCE);
	//Log::enable_exit_on_warning();
	//ngrid_test();
	//vkcontext_graphics_test_skull();
	vkcontext_graphics_test_helmet();

	std::cin.get();
}

