//
//  matrix.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_matrix_hpp
#define falling_matrix_hpp

#include "base/vector.hpp"

namespace falling {
	template <typename ElementType, size_t N /* width */, size_t M /* height */>
	struct TMatrix {
		// We use row-major notation.
		typedef TVector<ElementType, N> Row;
		typedef TVector<ElementType, M> Column;
		
		TMatrix() {}
		TMatrix(ArrayRef<ElementType> array);
		TMatrix(ArrayRef<Row> array);
		TMatrix(const TMatrix<ElementType, N, M>&) = default;
		TMatrix(TMatrix<ElementType, N, M>&&) = default;
		
		Row row_at(size_t idx) const;
		void set_row(size_t idx, Row row);
		Column column_at(size_t idx) const;
		
		// Convenience
		static TMatrix<ElementType,N,M> identity();
	private:
		Row rows_[M];
	};
	
	using matrix44 = TMatrix<float32, 4, 4>;
	using matrix43 = TMatrix<float32, 4, 3>;
	
	template <typename T, size_t N, size_t M>
	typename TMatrix<T, N, M>::Row TMatrix<T,N,M>::row_at(size_t idx) const {
		ASSERT(idx < M);
		// Fast.
		return rows_[idx];
	}
	
	template <typename T, size_t N, size_t M>
	void TMatrix<T,N,M>::set_row(size_t idx, Row row) {
		ASSERT(idx < M);
		rows_[idx] = row;
	}
	
	template <typename T, size_t N, size_t M>
	typename TMatrix<T, N, M>::Column TMatrix<T,N,M>::column_at(size_t idx) const {
		ASSERT(idx < N);
		// Goddamn slow.
		Column result;
		for (size_t i = 0; i < M; ++i) {
			result[i] = rows_[i][idx];
		}
		return result;
	}
	
	template <typename T, size_t N, size_t M>
	TMatrix<T,N,M> TMatrix<T,N,M>::identity() {
		TMatrix<T,N,M> result;
		Row v;
		v[0] = 1;
		size_t idx = 0;
		do {
			result.set_row(idx++, v);
			v = shift_right(v);
		} while (idx < M);
		return result;
	}
	
	
	template <typename T, size_t N, size_t M, size_t P>
	struct MatrixMultiplier;
	
	// General implementation
	template <typename T, size_t N, size_t M, size_t P>
	struct MatrixMultiplier {
		TMatrix<T, N, P>
		multiply(const TMatrix<T, N, M>& a, const TMatrix<T, M, P>& b) {
			TMatrix<T, N, P> result;
			for (size_t i = 0; i < M; ++i) {
				auto row = a.row_at(i);
				auto col = b.column_at(i);
				result.set_row(i, row.dot(col));
			}
			return result;
		}
	};
	
	// TODO: Optimized multiplier for 44, 43, 34, 33...
	
	template <typename ElementType, size_t N, size_t M, size_t P>
	TMatrix<ElementType, N, P>
	matrix_multiply(const TMatrix<ElementType, N, M>& a, const TMatrix<ElementType, M, P>& b) {
		return MatrixMultiplier<ElementType, N, M, P>::multiply(a, b);
	}
}

#endif
