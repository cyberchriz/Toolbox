#include "vknet.h"

int main() {
	vec inputs(10);
	vec outputs(10);
	VkNet network(&inputs, &outputs, &inputs, 10, 10, 1.0f);
	for (uint32_t i = 0; i < 10000; i++) {
		inputs.fill_random_uniform(0, 1);
		network.process(true, 0.05);
		if (i % 1000 == 0) {
			Log::log(FORCE_PLAIN, "\n", i, " iterations:");
			inputs.print("inputs: ", "|", false, true);
			outputs.print("outputs: ", "|", false, true);
		}
	}
}
