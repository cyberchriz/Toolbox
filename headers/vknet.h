#ifndef VKNET_H
#define VKNET_H

#include "vkvec.h"

class VkNet {
public:
	// constructor
	VkNet(uint32_t inputs, uint32_t outputs, uint32_t hidden, uint32_t hidden_connections = 10) {
		this->connections = hidden_connections;
		this->neurons = inputs + outputs + hidden;

		X = X.resize(neurons); // neuron input states
		X.fill_zero();
	
		H = H.resize(neurons); // neuron output states
		H.fill_random_gaussian();

		G = G.resize(neurons); // gradients
		G.fill_zero();
		
		Beta1 = Beta1.resize(neurons); // update speed for outputs H and gradients G
		Beta1.fill_random_gaussian(0, 0.05);
		Beta1 = Beta1.abs();

		Beta2 = Beta2.resize(neurons);
		Beta2 = (Beta1 * -1) + 1;
		
		IsInput = IsInput.resize(neurons); // boolean map to indicate whether a given neuron is an input neuron
		IsInput.fill_zero();
		input_index.resize(inputs);
		for (uint32_t i = 0; i < inputs; i++) {
			input_index[i] = i;
			IsInput.set(true, i);
		}
		
		IsOutput = IsOutput.resize(neurons); // boolean map to indicate whether a given neuron is an output neuron
		IsOutput.fill_zero();
		output_index.resize(outputs);
		for (uint32_t i = inputs; i < inputs + outputs; i++) {
			output_index[i - inputs] = i;
			IsOutput.set(true, i);
		}

		for (uint32_t i = 0; i < connections; i++) {
			// initialize weights
			W.push_back(VkVec(neurons));
			W[i].fill_Xavier_sigmoid(connections, connections);

			// set source neurons
			I.push_back(VkVec(neurons));
			I[i].fill_random_uniform_int(0, neurons - 1);
		}
	}

	// destructor
	~VkNet() {}

	void add_inputs(uint32_t amount) {
		X = X.add_rows(amount);
		H = H.add_rows(amount);
		G = G.add_rows(amount);
		Beta1 = Beta1.add_rows(amount);
		Beta2 = Beta2.add_rows(amount);
		IsInput = IsInput.add_rows(amount);
		IsOutput = IsOutput.add_rows(amount);
		for (uint32_t i = 0; i < amount; i++) {
			IsInput.set(true, neurons + i);
			IsOutput.set(false, neurons + i);
			input_index.push_back(neurons + i);
		}
		VkVec WeightInit(neurons + amount);
		for (uint32_t i = 0; i < connections; i++) {
			W[i] = W[i].add_rows(amount);
			WeightInit.fill_He_ReLU(connections);
			W[i] = W[i].replace_if(W[i] == 0, WeightInit);
		}
		neurons += amount;
	}

	void add_outputs(uint32_t amount) {
		X = X.add_rows(amount);
		H = H.add_rows(amount);
		G = G.add_rows(amount);
		Beta1 = Beta1.add_rows(amount);
		Beta2 = Beta2.add_rows(amount);
		IsInput = IsInput.add_rows(amount);
		IsOutput = IsOutput.add_rows(amount);
		for (uint32_t i = 0; i < amount; i++) {
			IsInput.set(false, neurons + i);
			IsOutput.set(true, neurons + i);
			output_index.push_back(neurons + i);
		}
		VkVec WeightInit(neurons + amount);
		for (uint32_t i = 0; i < connections; i++) {
			W[i] = W[i].add_rows(amount);
			WeightInit.fill_He_ReLU(connections);
			W[i] = W[i].replace_if(W[i] == 0, WeightInit);
		}
		neurons += amount;
	}

	void set_input(uint32_t input_neuron, float_t value) {
		X.set(value, input_index[input_neuron]);
	}

	void set_inputs(const std::vector<float_t> inputs) {
		for (uint32_t i = 0; i < inputs.size() && i < input_index.size(); i++) {
			X.set(inputs[i], input_index[i]);
		}
	}

	void set_inputs(const VkVec& inputs) {
		for (uint32_t i = 0; i < inputs.get_elements() && i < input_index.size(); i++) {
			X.set(inputs.get(i), input_index[i]);
		}
	}

	float_t get_output(uint32_t output_neuron) {
		return H.get(output_index[output_neuron]);
	}

	void set_label(uint32_t output_neuron, float_t value) {
		G.set(H.get(output_index[output_neuron]) - value, output_index[output_neuron]);
	}

	void set_labels(const std::vector<float_t> labels) {
		for (uint32_t i = 0; i < labels.size() && i < output_index.size(); i++) {
			G.set(H.get(output_index[i]) - labels[i], output_index[i]);
		}
	}

	void set_labels(const VkVec& labels) {
		for (uint32_t i = 0; i < labels.get_elements() && i < output_index.size(); i++) {
			G.set(H.get(output_index[i]) - labels.get(i), output_index[i]);
		}
	}

	VkVec get_outputs() {
		VkVec result(output_index.size());
		for (uint32_t i = 0; i < output_index.size(); i++) {
			result.set(H.get(output_index[i]), i);
		}
		return result;
	}

	void recover() {
		for (uint32_t i = 0; i < connections; i++) {
			W[i] = W[i].recover();
		}
		G = G.recover();
		H.recover();
	}

	void process(uint32_t iterations = 1) {
		for (uint32_t n = 0; n < iterations; n++) {

			// update input states X
			X = X.replace_if(!IsInput, 0);
			for (uint32_t i = 0; i < connections; i++) {
				VkVec condition_map; condition_map = !IsInput;
				VkVec replacing_map; replacing_map = X + (H.remap_to(I[i])).Hadamard_product(W[i]);
				X = X.replace_if(condition_map, replacing_map);
			}

			// update neuron outputs H
			H = X.activation(LRELU).Hadamard_product(Beta1) + H.Hadamard_product(Beta2);

			// propagate gradients
			// hidden gradient = SUM_k[err_k * w_jk]
			VkVec new_gradient(neurons);
			new_gradient.fill_zero();
			for (uint32_t i = 0; i < connections; i++) {
				new_gradient += G.remap_to(I[i]).Hadamard_product(W[i]);
			}
			G = G.replace_if(!IsOutput, new_gradient.Hadamard_product(Beta1) + G.Hadamard_division(Beta2));

			// update weights
			for (uint32_t i = 0; i < connections; i++) {
				// general rule: delta w_ij =  error_k * act'(net_inp_j) * out_i * lr
				W[i] -= G.Hadamard_product(X.derivative(LRELU)).Hadamard_product(H.remap_to(I[i])) * alpha;
			}
			X.print("X:", "|", false, true, -1);
			W[0].print("W[0]:", "|", false, true, -1);
			I[0].print("I[0]", "|", false, true, -1);
			H.print("H:", "|", false, true, -1);
			IsInput.print("IsInput:", "|", false, true, -1);
			IsOutput.print("IsOutput:", "|", false, true, -1);
			// remove any inf / nan
			recover();
		}
	}

protected:

private:
	VkVec X; // neuron input state
	VkVec H; // neuron output state
	VkVec G; // gradient
	VkVec Beta1; // update speed for output and error
	VkVec Beta2; // retained fraction for output and error, equal to 1 - Beta1
	VkVec IsInput; // boolean map to indicate whether a given neuron is an input neuron
	VkVec IsOutput; // boolean map to indicate whether a given neuron is an output neuron
	std::vector<VkVec> W; // incoming weights
	std::vector<VkVec> I; // index map of source neurons the weights W are coming from
	std::vector<uint32_t> input_index;
	std::vector<uint32_t> output_index;
	uint32_t connections = 0;
	uint32_t neurons = 0;
	float_t alpha = 0.0001;
};

#endif