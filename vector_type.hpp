//
//  vector_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_vector_type_hpp
#define falling_vector_type_hpp

#include "type/type.hpp"
#include "base/vector.hpp"
#include "serialization/archive_node.hpp"

namespace falling {
	struct VectorType : public SimpleType {
		VectorType(std::string name, size_t width, size_t component_width, bool is_float, bool is_signed = true) : SimpleType(name, width, component_width, is_float, is_signed) {}
	};
	
	template <typename T, size_t N>
	struct VectorTypeImpl : public TypeFor<TVector<T, N>, VectorType> {
		VectorTypeImpl(std::string name) : TypeFor<TVector<T,N>, VectorType>(std::move(name), sizeof(T)*N, sizeof(T), IsFloatingPoint<T>::Value, IsSigned<T>::Value) {}
		
		virtual void deserialize(TVector<T, N>&, const ArchiveNode&, IUniverse&) const override;
		virtual void serialize(const TVector<T,N>&, ArchiveNode&, IUniverse&) const override;
		void* cast(const SimpleType* to, void* o) const {
			if (to->size() == this->size() && to->num_components() == this->num_components()) {
				return o;
			}
			return nullptr;
		}
	};
	
	template <typename T, size_t N>
	void VectorTypeImpl<T,N>::deserialize(TVector<T, N>& vector, const ArchiveNode& node, IUniverse&) const {
		for (size_t i = 0; i < N; ++i) {
			const ArchiveNode& component = node[VectorComponentNames[i]];
			component.get(vector[i]);
		}
	}
	
	template <typename T, size_t N>
	void VectorTypeImpl<T,N>::serialize(const TVector<T, N>& vector, ArchiveNode& node, IUniverse&) const {
		for (size_t i = 0; i < N; ++i) {
			node[VectorComponentNames[i]] = vector[i];
		}
	}
	
	template <typename T, size_t N>
	struct BuildTypeInfo<TVector<T,N>> {
		static const VectorTypeImpl<T,N>* build_vector_type() {
			StringStream ss;
			if (!IsFloatingPoint<T>::Value) {
				if (IsSigned<T>::Value) {
					ss << 'i';
				} else {
					ss << 'u';
				}
			}
			ss << "vec" << N;
			return new VectorTypeImpl<T,N>(ss.str());
		}
		
		static const VectorTypeImpl<T,N>* build() {
			static const VectorTypeImpl<T,N>* type = build_vector_type();
			return type;
		}
	};
}

#endif
