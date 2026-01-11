#ifndef STRINGHASH_H
#define STRINGHASH_H

// Custom Key structure for optimized unordered_map lookups
struct StringHash {
	std::string string = "";
	uint64_t pre_calculated_hash = 0;

	StringHash(const std::string& hash_from_string) {
		try {
			string = std::filesystem::absolute(hash_from_string).string();
		}
		catch (const std::filesystem::filesystem_error& e) {
			string = hash_from_string;
		}
		pre_calculated_hash = std::hash<std::string>{}(string);
	}
	StringHash() : pre_calculated_hash(0) {}

	// Overload the equality operator for collision checking
	bool operator==(const StringHash& other) const {
		// 1. CHEAP CHECK: Compare the hash values first. If hashes differ, paths MUST be different.
		if (pre_calculated_hash != other.pre_calculated_hash) {
			return false;
		}

		// 2. EXPENSIVE CHECK (Only executed on hash collision): 
		// Compare the full strings to handle the rare hash collision scenario.
		return string == other.string;
	}
};

// Specialization of std::hash for the StringHash struct
// This tells the unordered_map how to get the hash for bucket lookup.
template <>
struct std::hash<StringHash> {
	size_t operator()(const StringHash& tp) const {
		return tp.pre_calculated_hash;
	}
};

#endif // STRINGHASH_H