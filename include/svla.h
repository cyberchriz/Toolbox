#ifndef SVLA_H
#define SVLA_H

#define SVLA_MAX_SIZE 65536
#define SVLA_MAX_REGIONS 128

// Declare the virtual base class (necessary for sharing of static members)
virtual class SVLA_Base {
protected:
	static uint32_t allocated_regions;
	static uchar8_t data[SVLA_MAX_SIZE];
	static uchar8_t* region_begin[SVLA_MAX_REGIONS];
	static uint32_t region_elements[SLVA_MAX_REGIONS];
	static size_t region_size[SVLA_MAX_REGIONS];
	static bool region_in_use[SVLA_MAX_REGIONS];
};

// initialize static members outside class definition
uint32_t SVLA_Base::allocated_regions = 0;
uchar8_t SVLA_Base::data[SVLA_MAX_SIZE] = { 0 };
uchar8_t* SVLA_Base::region_begin[SVLA_MAX_REGIONS] = { nullptr };
uint32_t SVLA_Base::region_elements[SVLA_MAX_REGIONS] = { 0 };
size_t SVLA_Base::region_size[SVLA_MAX_REGIONS] = { SVLA_MAX_SIZE };
bool SVLA_Base::region_in_use[SVLA_MAX_REGIONS] = { false };


// stack-allocated variable-length array
template<typename T, int N>
class SVLA : public SVLA_Base {
public:
	// default constructor
	SVLA() {
		region_elements[region_id] = N;
		region_size[region_id] = N * sizeof(T);
		region_in_use[region_id] = true;
		region_begin[region_id] = region_id > 0 ? region_begin[region_id - 1] + region_size[region_id - 1] : &data;
		if (region_begin[region_id] + region_size[region_id] > &data + SVLA_MAX_SIZE) {
			region_begin[region_id] = &data + defragment_and_allocate(region_size[region_id]);
		}
	};

	// destructor
	~SVLA() {
		this->clear();
	};

	// subscript operator overload (for array-like access)
	T& operator[](uint32_t index) {
		return reinterpret_cast<T*>(region_begin[region_id])[index];
	}

	T* begin() const {
		return reinterpret_cast<T*>(region_begin[region_id]);
	}

	T* end() const {
		return reinterpret_cast<T*>(region_begin[region_id] + region_size[region_id]);
	}

	void resize(uint32_t elements_count) {
		region_elements[region_id] = elements_count;
		size_t new_size = elements_count * sizeof(T);

		// simply shrink the current region if new size is smaller
		if (new_size < region_size[region_id]) {
			region_size[region_id] = new_size;
			return;
		}
		
		// allocate new region if new size is larger
		else {
			// check for available space
			uint32_t last_region_id = allocated_regions > 0 ? allocated_regions - 1 : 0;
			size_t new_offset = region_begin[last_region_id] + region_size[last_region_id];
			// deframent the data buffer in case the new allocation doesn't fit
			if (new_offset + new_size > SVLA_MAX_SIZE) {
				defragment();
			}
		}
		region_size[region_id] = new_size;
	}

	void clear() {
		if (region_in_use[region_id]) {
			region_in_use[region_id] = false;
			allocated_regions_in_use--;
		}
	}
private:
	void defragment() {
		uchar8_t* current_position = &data;
		for (uint32_t i = 0; i < SVLA_MAX_REGIONS; i++) {
			if (region_in_use[i]) {
				if (region_begin[i] != current_position) {
					memmove(current_position, region_begin[i], region_size[i]);
					region_begin[i] = current_position;
				}
				current_position += region_size[i];
			}
		}
	}

	template<typename T, int N> uchar8_t* allocate(size_t allocation_size) {
		for (uint32_t i = 0; i < SVLA_MAX_REGIONS; i++) {
			if (!region_in_use[i] && region_size[i] >= allocation_size) {
				region_begin[i] = i > 0 ? region_begin[i - 1] + region_size[i - 1] : &data;
				if (region_begin[i] + allocation_size > &data + SVLA_MAX_SIZE) {
					defragment();
					region_begin[i] = i > 0 ? region_begin[i - 1] + region_size[i - 1] : &data;
				}
				region_in_use[i] = true;
				region_size[i] = allocation_size;
				allocated_regions++;
				return region_begin[i];
			}
		}
	}

	uint32_t region_id;
};

#endif