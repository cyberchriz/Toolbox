#ifndef NNet_H
#define NNet_H

#include <log.h>
#include <ngrid.h>

// Available Neural Network Layer Types
enum LayerType {
	LAYER_TYPE_INPUT,
	LAYER_TYPE_DENSE,
	LAYER_TYPE_SPARSE,
	LAYER_TYPE_RELU,
	LAYER_TYPE_TANH,
	LAYER_TYPE_SIGMOID,
	LAYER_TYPE_FLATTEN,
	LAYER_TYPE_CONVOLUTION,
	LAYER_TYPE_LSTM,
	LAYER_TYPE_GRU,
	LAYER_TYPE_SOFTMAX
};

// NNetLayer struct declaration
struct NNetLayer {
	// constructors / destructors
	NNetLayer() {};
	~NNetLayer();

	// methods
	uint32_t neurons() const { return state == nullptr ? 0 : state->get_elements; }
	const std::vector<uint32_t>& shape() const { return state == nullptr ? std::vector<uint32_t>({}) : state->get_shape(); }

	// member variables
	NGrid* state = nullptr;
	NGrid* error = nullptr;
	NGrid* dense_weights = nullptr;
	NGrid* sparse_weights = nullptr;
	NGrid* sparse_source_neurons = nullptr;
	LayerType type;
	float_t relu_alpha = 0;
	uint32_t sparse_inputs_per_neuron = 0;
};

// NNet class declaration
class NNet {
	friend class NNetLayer;
public:
	// +=================================+   
	// | Constructors / Destructors      |
	// +=================================+
	NNet();
	~NNet();

	// +=================================+   
	// | Add Layers                      |
	// +=================================+
	// these methods return a reference to 'this' in order to allow method chaining ('fluent interface')
	NNet& input(std::initializer_list<uint32_t> input_shape);
	NNet& dense(std::initializer_list<uint32_t> shape);
	NNet& sparse(std::initializer_list<uint32_t> shape, uint32_t inputs_per_neuron);
	NNet& relu(float_t alpha = 0.01f);
	NNet& tanh();
	NNet& sigmoid();
	NNet& flatten();
	NNet& convolution();
	NNet& lstm();
	NNet& gru();
	NNet& softmax();

	// +=================================+   
	// | Execution                       |
	// +=================================+
	NGrid predict(const NGrid& sample);
	void train(const NGrid& sample, const NGrid& label);
	void train(const std::vector<NGrid>& sample_batch, const std::vector<NGrid>& label_batch);
	void test(const std::vector<NGrid>& test_sample_batch, const std::vector<NGrid>& test_label_batch);
private:
	void init_weights();
	bool weights_initialized = false;
	std::vector<NNetLayer> layers;
	uint32_t num_layers() const { return static_cast<uint32_t>(layers.size()); }
};

// ==============================================================================================================================
// DEFINITIONS

// NNetLayer destructor
NNetLayer::~NNetLayer() {
	if (state != nullptr) { delete state; state = nullptr; }
	if (error != nullptr) { delete error; error = nullptr; }
	if (dense_weights != nullptr) { delete dense_weights; dense_weights = nullptr; }
	if (sparse_weights != nullptr) { delete sparse_weights; sparse_weights = nullptr; }
	if (sparse_source_neurons != nullptr) { delete sparse_source_neurons; sparse_source_neurons = nullptr; }
}

// +=================================+   
// | Add Layers                      |
// +=================================+

// create an input layer for the neural network;
// note: input layers only have a shape and states;
// as the final layer during backpropagation, they don't require an error buffer;
// as the first layer during feed-forward, they don't require any input weights
NNet& NNet::input(std::initializer_list<uint32_t> input_shape) {
	uint32_t layer_index = this->num_layers();
	if (layer_index != 0) {
		Log::warning("Invalid call of method NNet::input(): the network already has ", num_layers(), " layers. Only the first layer can be an input layer.");
		return *this;
	}
	layers.push_back(NNetLayer);
	layers[layer_index].type = LayerType::LAYER_TYPE_INPUT;
	layers[layer_index].state = new NGrid(input_shape);
	return *this;
}

NNet& NNet::dense(std::initializer_list<uint32_t> shape) {
	uint32_t layer_index = this->num_layers();
	if (layer_index == 0) {
		Log::warning("Invalid call of method NNet::dense(): the network has no other layers yet. Always add input layer first!");
		return *this;
	}
	layers.push_back(NNetLayer);
	layers[layer_index].type = LayerType::LAYER_TYPE_DENSE;
	layers[layer_index].state = new NGrid(shape);
	layers[layer_index].error = new NGrid(shape);
	uint32_t neurons_j = layers[layer_index].neurons();
	uint32_t neurons_i = layers[layer_index - 1].neurons();
	layers[layer_index].dense_weights = new NGrid({ neurons_i, neurons_j });
	return *this;
}

NNet& NNet::sparse(std::initializer_list<uint32_t> shape, uint32_t inputs_per_neuron) {
	uint32_t layer_index = this->num_layers();
	if (layer_index == 0) {
		Log::warning("Invalid call of method NNet::sparse(): the network has no other layers yet. Always add input layer first!");
		return *this;
	}
	layers.push_back(NNetLayer);
	layers[layer_index].type = LayerType::LAYER_TYPE_SPARSE;
	layers[layer_index].state = new NGrid(shape);
	layers[layer_index].error = new NGrid(shape);
	uint32_t neurons_j = layers[layer_index].neurons();
	uint32_t neurons_i = layers[layer_index - 1].neurons();
	layers[layer_index].sparse_weights = new NGrid({ inputs_per_neuron, neurons_j });
	layers[layer_index].sparse_source_neurons = new NGrid({ inputs_per_neuron, neurons_j });
	layers[layer_index].sparse_source_neurons->fill_random_int(0, neurons_i - 1);
	layers[layer_index].sparse_inputs_per_neuron = inputs_per_neuron;
	return *this;
}

NNet& NNet::relu(float_t alpha) {
	uint32_t layer_index = this->num_layers();
	if (layer_index == 0) {
		Log::warning("Invalid call of method NNet::relu(): the network has no other layers yet. Always add input layer first!");
		return *this;
	}
	layers.push_back(NNetLayer);
	layers[layer_index].type = LayerType::LAYER_TYPE_RELU;
	layers[layer_index].state = new NGrid(layers[layer_index - 1].shape());
	layers[layer_index].error = new NGrid(layers[layer_index - 1].shape());
	layers[layer_index].relu_alpha = alpha;

	return *this;
}

NNet& NNet::tanh() {
	uint32_t layer_index = this->num_layers();
	if (layer_index == 0) {
		Log::warning("Invalid call of method NNet::tanh(): the network has no other layers yet. Always add input layer first!");
		return *this;
	}
	layers.push_back(NNetLayer);
	layers[layer_index].type = LayerType::LAYER_TYPE_RELU;
	layers[layer_index].state = new NGrid(layers[layer_index - 1].shape());
	layers[layer_index].error = new NGrid(layers[layer_index - 1].shape());

	return *this;
}

NNet& NNet::sigmoid() {
	return *this;
}

NNet& NNet::flatten() {
	return *this;
}

NNet& NNet::convolution() {
	return *this;
}

NNet& NNet::lstm() {
	return *this;
}

NNet& NNet::gru() {
	return *this;
}

NNet& NNet::softmax() {
	return *this;
}

#endif // NNet_H