//
//  matrix_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_matrix_type_hpp
#define grace_matrix_type_hpp

#include "base/matrix.hpp"
#include "type/type.hpp"
#include "base/log.hpp"

namespace grace {
	struct MatrixType : public SimpleType {
		MatrixType(const TypeInfo& ti, IAllocator& alloc, StringRef name, size_t width, size_t component_width, bool is_float, bool is_signed = true) : SimpleType(alloc, ti, std::move(name), width, component_width, is_float, is_signed) {}
		virtual size_t num_columns() const = 0;
		virtual size_t num_rows() const = 0;
		virtual void* cast(const SimpleType* to, void* o) const { ASSERT(false); return nullptr; /* NIY */ }
	protected:
		String build_type_name_for_matrix(IAllocator& alloc, size_t cols, size_t rows);
	};
	
	template <typename T, size_t N, size_t M>
	struct MatrixTypeImpl : public TypeFor<TMatrix<T,N,M>, MatrixType> {
		MatrixTypeImpl(IAllocator& alloc) : TypeFor<TMatrix<T,N,M>, MatrixType>(alloc, this->build_type_name_for_matrix(alloc, N,M), N*M, sizeof(T), IsFloatingPoint<T>::Value, IsSigned<T>::Value) {}
		size_t num_columns() const { return N; }
		size_t num_rows() const { return M; }
		
		virtual void deserialize(TMatrix<T,N,M>& place, const DocumentNode&, IUniverse&) const;
		virtual void serialize(const TMatrix<T,N,M>& place, DocumentNode&, IUniverse&) const;
	};
	
	template <typename T, size_t N, size_t M>
	struct BuildTypeInfo<TMatrix<T,N,M>> {
		static const MatrixTypeImpl<T,N,M>* build() {
			static const auto p = new_static MatrixTypeImpl<T,N,M>(static_allocator());
			return p;
		}
	};
	
	template <typename T, size_t N, size_t M>
	void MatrixTypeImpl<T,N,M>::deserialize(TMatrix<T, N, M> &place, const DocumentNode& node, IUniverse&) const {
		if (node.is_array() && node.array_size() >= M) {
			for (size_t r = 0; r < M; ++r) {
				const DocumentNode& row = node[r];
				if (row.is_array() && node.array_size() >= N) {
					for (size_t c = 0; c < N; ++c) {
						const DocumentNode& element = row[c];
						T x;
						if (element >> x) {
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
	void MatrixTypeImpl<T,N,M>::serialize(const TMatrix<T, N, M>& place, DocumentNode& node, IUniverse&) const {
		for (size_t r = 0; r < M; ++r) {
			DocumentNode& row = node.array_push();
			for (size_t c = 0; c < N; ++c) {
				DocumentNode& column = row.array_push();
				column << place.row_at(r)[c];
			}
		}
	}
}

#endif
