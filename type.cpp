#include "type/type.hpp"
#include "serialization/archive_node.hpp"
#include <map>

namespace falling {

#define DEFINE_SIMPLE_TYPE(T, IS_FLOAT, IS_SIGNED) \
const SimpleType* BuildTypeInfo<T>::build() {\
	if (IS_FLOAT) { \
		static const FloatType* t = new_static FloatType(#T, sizeof(T)); \
		return t; \
	} else { \
		static const IntegerType* t = new_static IntegerType(#T, sizeof(T)); \
		return t; \
	} \
}

DEFINE_SIMPLE_TYPE(int8, false, true)
DEFINE_SIMPLE_TYPE(int16, false, true)
DEFINE_SIMPLE_TYPE(int32, false, true)
DEFINE_SIMPLE_TYPE(int64, false, true)
DEFINE_SIMPLE_TYPE(uint8, false, false)
DEFINE_SIMPLE_TYPE(uint16, false, false)
DEFINE_SIMPLE_TYPE(uint32, false, false)
DEFINE_SIMPLE_TYPE(uint64, false, false)
DEFINE_SIMPLE_TYPE(float32, true, true)
DEFINE_SIMPLE_TYPE(float64, true, true)


void IntegerType::deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase&) const {
	if (is_signed_) {
		switch (width_) {
			case 1: node.get(*reinterpret_cast<int8* >(place)); return;
			case 2: node.get(*reinterpret_cast<int16*>(place)); return;
			case 4: node.get(*reinterpret_cast<int32*>(place)); return;
			case 8: node.get(*reinterpret_cast<int64*>(place)); return;
			default: ASSERT(false); // non-standard integer size
		}
	} else {
		switch (width_) {
			case 1: node.get(*reinterpret_cast<uint8* >(place)); return;
			case 2: node.get(*reinterpret_cast<uint16*>(place)); return;
			case 4: node.get(*reinterpret_cast<uint32*>(place)); return;
			case 8: node.get(*reinterpret_cast<uint64*>(place)); return;
			default: ASSERT(false); // non-standard integer size
		}
	}
}

void IntegerType::serialize_raw(const byte* place, ArchiveNode& node, UniverseBase&) const {
	if (is_signed_) {
		switch (width_) {
			case 1: node.set(*reinterpret_cast<const int8* >(place)); return;
			case 2: node.set(*reinterpret_cast<const int16*>(place)); return;
			case 4: node.set(*reinterpret_cast<const int32*>(place)); return;
			case 8: node.set(*reinterpret_cast<const int64*>(place)); return;
			default: ASSERT(false); // non-standard integer size
		}
	} else {
		switch (width_) {
			case 1: node.set(*reinterpret_cast<const uint8* >(place)); return;
			case 2: node.set(*reinterpret_cast<const uint16*>(place)); return;
			case 4: node.set(*reinterpret_cast<const uint32*>(place)); return;
			case 8: node.set(*reinterpret_cast<const uint64*>(place)); return;
			default: ASSERT(false); // non-standard integer size
		}
	}
}

void* IntegerType::cast(const SimpleType* to, void* memory) const {
	if (to == this) return memory;
	
	auto enum_type = dynamic_cast<const EnumType*>(to);
	if (enum_type != nullptr && enum_type->size() <= width_) {
		if (is_signed_) {
			ASSERT(width_ <= sizeof(ssize_t));
			ssize_t n = 0;
			memcpy(&n, memory, width_);
			if (enum_type->contains(n))
				return memory;
		} else {
			ASSERT(width_ <= sizeof(size_t));
			size_t n = 0;
			memcpy(&n, memory, width_);
			if (enum_type->contains(n))
				return memory;
		}
	}
	
	return nullptr;
}

void FloatType::deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase&) const {
	if (width_ == 4) {
		node.get(*reinterpret_cast<float32*>(place));
		return;
	} else if (width_ == 8) {
		node.get(*reinterpret_cast<float64*>(place));
		return;
	}
	ASSERT(false); // FloatType with neither 32-bit nor 64-bit floats?
}

void FloatType::serialize_raw(const byte* place, ArchiveNode& node, UniverseBase&) const {
	if (width_ == 4) {
		node.set(*reinterpret_cast<const float32*>(place));
	} else if (width_ == 8) {
		node.set(*reinterpret_cast<const float64*>(place));
	}
	ASSERT(false); // FloatType with neither 32-bit nor 64-bit floats?
}

bool EnumType::contains(ssize_t value) const {
	if (value >= min() && value <= max()) {
		for (auto& tuple: entries_) {
			if (std::get<1>(tuple) == value)
				return true;
		}
	}
	return false;
}

bool EnumType::name_for_value(ssize_t value, String& name) const {
	if (value >= min() && value <= max()) {
		for (auto& tuple: entries_) {
			if (std::get<1>(tuple) == value) {
				name = std::get<0>(tuple);
				return true;
			}
		}
	}
	return false;
}

bool EnumType::value_for_name(const String& name, ssize_t& out_value) const {
	for (auto& tuple: entries_) {
		if (std::get<0>(tuple) == name) {
			out_value = std::get<1>(tuple);
			return true;
		}
	}
	return false;
}

void EnumType::deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase&) const {
	String name;
	if (node.get(name)) {
		ssize_t value;
		ASSERT(width_ <= sizeof(ssize_t));
		if (value_for_name(name, value)) {
			memcpy(place, &value, width_);
			// Success!
		} else {
			// XXX: Invalid enum entry.
		}
	} else {
		// XXX: Invalid node (not a string)
	}
}

void EnumType::serialize_raw(const byte* place, ArchiveNode& node, UniverseBase&) const {
	ssize_t value = 0;
	ASSERT(width_ <= sizeof(ssize_t));
	memcpy(&value, place, width_);
	String name;
	if (name_for_value(value, name)) {
		node.set(name);
		// Success!
	} else {
		// XXX: Invalid enum entry!
	}
}

void* EnumType::cast(const SimpleType* to, void* memory) const {
	if (to == this) return memory;
	
	auto integer_type = dynamic_cast<const IntegerType*>(to);
	if (integer_type != nullptr && integer_type->size() <= width_) {
		if (integer_type->is_signed()) {
			ssize_t value = 0;
			ASSERT(integer_type->size() <= sizeof(ssize_t));
			memcpy(&value, memory, integer_type->size());
			if (contains(value)) {
				return memory;
			}
		} else {
			size_t value = 0;
			ASSERT(integer_type->size() <= sizeof(size_t));
			memcpy(&value, memory, integer_type->size());
			if (contains(value)) {
				return memory;
			}
		}
	}
	
	return nullptr;
}

void* FloatType::cast(const SimpleType* to, void* memory) const {
	if (to == this) return memory;
	return nullptr;
}

const char VoidType::Name[] = "void";

const VoidType* VoidType::get() {
	static const VoidType p;
	return &p;
}


void StringType::deserialize(String& place, const ArchiveNode& node, UniverseBase&) const {
	node.get(place);
}

void StringType::serialize(const String& place, ArchiveNode& node, UniverseBase&) const {
	node.set(place);
}

const StringType* StringType::get() {
	static const StringType type = StringType();
	return &type;
}

String StringType::name() const {
	return "String";
}

}
