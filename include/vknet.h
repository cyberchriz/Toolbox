// author and copyright: Christian Suer (cyberchriz)
// description: neural network class using GPU computation (with Vulkan), with self-organizing architecture

#ifndef VKNET_H
#define VKNET_H

#pragma once
#include <ngrid.h>

// forward declarations
class VkNet;

class VkNet {
public:
	// constructors & destructors
	VkNet(NGrid<float_t>* inputs, NGrid<float>* outputs, NGrid<float>* labels, const uint32_t hidden_neurons, const uint32_t weights_per_neuron = 10, const float min_update = 0.001);
	~VkNet();

	// processing
	void process(bool train = false, const float_t learning_rate = 0.001);


protected:
	void destroy();

	static VkManager* manager;
	static DescriptorPool* descriptor_pool;
	NGrid<float>* inputs = nullptr;
	NGrid<float>* outputs = nullptr;
	NGrid<float>* labels = nullptr;
	NGrid<float>* memory_states = nullptr;
	NGrid<float>* update_states = nullptr;
	NGrid<float>* activated_states = nullptr;
	NGrid<float>* errors = nullptr;
	NGrid<float>* weights = nullptr;
	NGrid<float>* weight_targets = nullptr;
	NGrid<float>* update_factors = nullptr;
	CommandBuffer* command_buffer = nullptr;
	uint32_t inputs_count = 0;
	uint32_t outputs_count = 0;
	uint32_t neurons_count = 0;

};

// DEFINITIONS
// ===============================================================================================================================

// +=================================+   
// | Static Member Initializations   |
// +=================================+

DescriptorPool* VkNet::descriptor_pool = nullptr;
VkManager* VkNet::manager = nullptr;


// +=================================+   
// | Constructors & Destructors      |
// +=================================+

// constructor
VkNet::VkNet(NGrid<float>* inputs, NGrid<float_t>* outputs, NGrid<float_t>* labels, const uint32_t hidden_neurons, const uint32_t weights_per_neuron, const float min_update) {
	
	manager = VkManager::get_singleton();
	descriptor_pool = NGrid<float_t>::descriptor_pool;
	command_buffer = new CommandBuffer(manager->get_device(), QueueFamily::COMPUTE, manager->get_command_pool_compute());

	this->inputs = inputs;
	inputs_count = inputs->get_elements();
	
	this->outputs = outputs;
	outputs_count = outputs->get_elements();

	this->labels = labels;

	neurons_count = inputs_count + outputs_count + hidden_neurons;
	
	memory_states = new NGrid<float_t>(neurons_count);
	memory_states->fill_zero();

	update_states = new NGrid<float_t>(neurons_count);

	activated_states = new NGrid<float_t>(neurons_count);

	update_factors = new NGrid<float_t>(neurons_count);
	update_factors->fill_random_uniform(std::max(std::min(1.0f, std::abs(min_update)), 0.0f), 1.0f);

	errors = new NGrid<float_t>(neurons_count);
	errors->fill_zero();

	weights = new NGrid<float_t>(neurons_count, weights_per_neuron);
	weights->fill_random_gaussian(0, 1);

	weight_targets = new NGrid(neurons_count, weights_per_neuron);
	weight_targets->fill_random_uniform_int(0, neurons_count - 1);
}

// destructor
VkNet::~VkNet() {
	this->destroy();
}

// protected helper method for object destruction
void VkNet::destroy() {
	if (this->memory_states != nullptr) {
		delete memory_states;
		memory_states = nullptr;
	}
	if (this->update_states != nullptr) {
		delete update_states;
		update_states = nullptr;
	}

	if (this->activated_states != nullptr) {
		delete activated_states;
		activated_states = nullptr;
	}
	if (this->update_factors != nullptr) {
		delete update_factors;
		update_factors = nullptr;
	}
	if (errors != nullptr) {
		delete errors;
		errors = nullptr;
	}
	if (weights != nullptr) {
		delete weights;
		weights = nullptr;
	}
	if (weight_targets != nullptr) {
		delete weight_targets;
		weight_targets = nullptr;
	}
	if (command_buffer != nullptr) {
		delete this->command_buffer;
		this->command_buffer = nullptr;
	}
}

void VkNet::process(bool train, const float_t learning_rate) {
	static constexpr uint32_t workgroup_size = 256;
	const uint32_t workgroups = neurons_count / workgroup_size + 1;

	static ShaderModule shader(manager->get_device());
	if (!shader.get()) { shader.read_from_file("vknet_process.spv"); }

	Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

	static std::vector<DescriptorType> types = {
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER,
		STORAGE_BUFFER
	};
	DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
	descriptor_set.bind_buffer(*memory_states->get_data_buffer(), 0);
	descriptor_set.bind_buffer(*inputs->get_data_buffer(), 1);
	descriptor_set.bind_buffer(*outputs->get_data_buffer(), 2);
	descriptor_set.bind_buffer(*labels->get_data_buffer(), 3);
	descriptor_set.bind_buffer(*weights->get_data_buffer(), 4);
	descriptor_set.bind_buffer(*weight_targets->get_data_buffer(), 5);
	descriptor_set.bind_buffer(*errors->get_data_buffer(), 6);
	descriptor_set.bind_buffer(*update_factors->get_data_buffer(), 7);
	descriptor_set.bind_buffer(signal, 8);
	descriptor_set.bind_buffer(*update_states->get_data_buffer(), 9);
	descriptor_set.bind_buffer(*activated_states->get_data_buffer(), 10);

	PushConstants push_constants;
	push_constants.add_value(uint32_t(train));
	push_constants.add_value(inputs_count);
	push_constants.add_value(outputs_count);
	push_constants.add_value(neurons_count);
	push_constants.add_value(weights->get_cols());
	push_constants.add_value(learning_rate);
	push_constants.add_value(seed32());

	ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
	command_buffer->compute(pipeline, neurons_count, 1, 1, workgroup_size);

	pipeline.destroy();
	descriptor_set.free();
}

#endif