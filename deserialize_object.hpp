#pragma once
#ifndef DESERIALIZE_OBJECT_HPP_F2934JFR
#define DESERIALIZE_OBJECT_HPP_F2934JFR

#include "object/objectptr.hpp"
#include "serialization/binary_archive.hpp"

namespace falling {
struct IUniverse;
struct UniverseBase;
struct ArchiveNode;

ObjectPtr<> deserialize_object(const ArchiveNode& representation, IUniverse& universe);
void deserialize_object(ObjectPtr<> place, const ArchiveNode& representation, IUniverse& universe);


void merge_object_templates(BinaryArchive& out_target, const ArchiveNode& object_definition);
const StructuredType* get_or_create_object_type(const ArchiveNode& object_definition, UniverseBase* universe);

}

#endif /* end of include guard: DESERIALIZE_OBJECT_HPP_F2934JFR */
