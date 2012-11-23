#pragma once
#ifndef SERIALIZE_HPP_37QGG4TA
#define SERIALIZE_HPP_37QGG4TA

#include "serialization/archive.hpp"
#include "object/object_type.hpp"

namespace falling {
	
template <typename T>
void serialize(const T& object, ArchiveNode& node, UniverseBase& universe) {
	const byte* memory = reinterpret_cast<const byte*>(&object);
	get_type(object)->serialize_raw(memory, node, universe);
}

}

#endif /* end of include guard: SERIALIZE_HPP_37QGG4TA */
