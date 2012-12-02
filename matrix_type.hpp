//
//  matrix_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 03/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_matrix_type_hpp
#define falling_matrix_type_hpp

#include "base/matrix.hpp"
#include "type/type.hpp"
#include "base/log.hpp"

namespace falling {
	struct MatrixType : public SimpleType {
		MatrixType(String name, size_t width, size_t component_width, bool is_float, bool is_signed = true) : SimpleType(std::move(name), width, component_width, is_float, is_signed) {}
		virtual size_t num_columns() const = 0;
		virtual size_t num_rows() const = 0;
		virtual void* cast(const SimpleType* to, void* o) const { ASSERT(false); return nullptr; /* NIY */ }
	protected:
		String build_type_name_for_matrix(size_t cols, size_t rows);
	};
	
	template <typename T, size_t N, size_t M>
	struct MatrixTypeImpl : public TypeFor<TMatrix<T,N,M>, MatrixType> {
		MatrixTypeImpl() : TypeFor<TMatrix<T,N,M>, MatrixType>(this->build_type_name_for_matrix(N,M), N*M, sizeof(T), IsFloatingPoint<T>::Value, IsSigned<T>::Value) {}
		size_t num_columns() const { return N; }
		size_t num_rows() const { return M; }
		
		virtual void deserialize(TMatrix<T,N,M>& place, const ArchiveNode&, UniverseBase&) const;
		virtual void serialize(const TMatrix<T,N,M>& place, ArchiveNode&, UniverseBase&) const;
	};
	
	template <typename T, size_t N, size_t M>
	struct BuildTypeInfo<TMatrix<T,N,M>> {
		static const MatrixTypeImpl<T,N,M>* build() {
			static const auto p = new_static MatrixTypeImpl<T,N,M>();
			return p;
		}
	};
	
	template <typename T, size_t N, size_t M>
	void MatrixTypeImpl<T,N,M>::deserialize(TMatrix<T, N, M> &place, const ArchiveNode& node, UniverseBase&) const {
		if (node.is_array() && node.array_size() >= M) {
			for (size_t r = 0; r < M; ++r) {
				const ArchiveNode& row = node[r];
				if (row.is_array() && node.array_size() >= N) {
					for (size_t c = 0; c < N; ++c) {
						const ArchiveNode& element = row[c];
						T x;
						if (element.get(x)) {
							place.row_at(r)[c] = x;
						} else {
							Warning() << "Could not deserialize matrix element.";
						}
					}
				} else {
					Warning() << "Could not deserialize matrix row (not an array, or too small).";
				}
			}
		} else {
			Warning() << "Could not deserialize matrix (not an array, or too small).";
		}
	}
	
	template <typename T, size_t N, size_t M>
	void MatrixTypeImpl<T,N,M>::serialize(const TMatrix<T, N, M>& place, ArchiveNode& node, UniverseBase&) const {
		for (size_t r = 0; r < M; ++r) {
			ArchiveNode& row = node.array_push();
			for (size_t c = 0; c < N; ++c) {
				ArchiveNode& column = row.array_push();
				column.set(place.row_at(r)[c]);
			}
		}
	}
}

#endif
