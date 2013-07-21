#pragma once
#ifndef DESERIALIZE_OBJECT_HPP_F2934JFR
#define DESERIALIZE_OBJECT_HPP_F2934JFR

#include "object/objectptr.hpp"
#include "serialization/binary_document.hpp"

namespace grace {
struct IUniverse;
struct UniverseBase;
struct DocumentNode;

ObjectPtr<> deserialize_object(const DocumentNode& representation, IUniverse& universe);
void deserialize_object(ObjectPtr<> place, const DocumentNode& representation, IUniverse& universe);


void merge_object_templates(Document& out_target, const DocumentNode& object_definition);
const StructuredType* get_or_create_object_type(const DocumentNode& object_definition, UniverseBase* universe);

}

#endif /* end of include guard: DESERIALIZE_OBJECT_HPP_F2934JFR */
