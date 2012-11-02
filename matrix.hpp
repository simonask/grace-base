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
#include <array>
#include <initializer_list>

namespace falling {
	template <typename ElementType, size_t N /* width */, size_t M /* height */>
	struct TMatrix {
		// We use row-major notation.
		typedef TMatrix<ElementType, N, M> Self;
		typedef TVector<ElementType, N> Row;
		typedef TVector<ElementType, M> Column;
		
		size_t height() const { return M; }
		size_t width() const { return N; }
		
		Row row_at(size_t idx) const;
		void set_row(size_t idx, Row row);
		
		Column column_at(size_t idx) const;
		void set_column(size_t idx, Column col);
		
		static Self from_rows(ArrayRef<Row> rows);
		static Self from_rows(ArrayRef<ElementType> row_elements);
		static Self from_rows(std::initializer_list<Row> rows);
		static Self from_rows(std::initializer_list<ElementType> row_elements);
		static Self from_columns(ArrayRef<Column> columns);
		static Self from_columns(ArrayRef<ElementType> column_elements);
		
		template <size_t N_ = N>
		static typename std::enable_if<N_ == M, TMatrix<ElementType, N_, N_>>::type
		identity();
	private:
		std::array<Row, M> rows_;
	};
	
	using matrix44 = TMatrix<float32, 4, 4>;
	using matrix43 = TMatrix<float32, 4, 3>;
	using matrix33 = TMatrix<float32, 3, 3>;
	using matrix22 = TMatrix<float32, 2, 2>;
	
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
	void TMatrix<T,N,M>::set_column(size_t idx, Column col) {
		ASSERT(idx < N);
		for (size_t i = 0; i < N; ++i) {
			rows_[i][idx] = col[i];
		}
	}
	
	template <typename T, size_t N> struct ComputeIdentity;
	template <typename T> struct ComputeIdentity<T, 2> {
		static TMatrix<T, 2, 2> compute() {
			return TMatrix<T,2,2>::from_rows({
				1, 0,
				0, 1,
			});
		}
	};
	
	template <typename T> struct ComputeIdentity<T, 3> {
		static TMatrix<T, 3, 3> compute() {
			return TMatrix<T,3,3>::from_rows({
				1, 0, 0,
				0, 1, 0,
				0, 0, 1,
			});
		}
	};
	
	template <typename T, size_t N> struct ComputeIdentity;
	template <typename T> struct ComputeIdentity<T, 4> {
		static TMatrix<T, 4, 4> compute() {
			return TMatrix<T,4,4>::from_rows({
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1,
			});
		}
	};
	
	template <typename T, size_t N, size_t M>
	template <size_t N_>
	typename std::enable_if<N_ == M, TMatrix<T,N_,N_>>::type
	inline TMatrix<T,N,M>::identity() {
		static Self m = ComputeIdentity<T, N>::compute();
		return m;
	}
	
	template <typename T, size_t N, size_t M>
	TMatrix<T,N,M> TMatrix<T,N,M>::from_rows(std::initializer_list<Row> rows) {
		ASSERT(rows.size() >= M); // Not enough elements
		Self m;
		auto it = rows.begin();
		for (size_t row = 0; row < M; ++row) {
			m.rows_[row] = *it;
			++it;
		}
		return m;
	}
	
	template <typename T, size_t N, size_t M>
	TMatrix<T,N,M> TMatrix<T,N,M>::from_rows(std::initializer_list<T> row_elements) {
		ASSERT(row_elements.size() >= N*M); // Not enough elements
		Self m;
		auto it = row_elements.begin();
		for (size_t row = 0; row < M; ++row) {
			for (size_t col = 0; col < N; ++col) {
				m.rows_[row][col] = *it;
				++it;
			}
		}
		return m;
	}
	
	
	template <typename T, size_t N, size_t M, size_t P>
	struct MatrixMultiplier;
	
	// General implementation
	template <typename T, size_t N, size_t M, size_t P>
	struct MatrixMultiplier {
		static TMatrix<T, N, P>
		multiply(const TMatrix<T, N, M>& a, const TMatrix<T, P, N>& b) {
			TMatrix<T, N, P> result;
			for (size_t col = 0; col < P; ++col) {
				auto column = b.column_at(col);
				for (size_t row = 0; row < M; ++row) {
					auto r = a.row_at(row);
					auto products = column * r;
					auto sum = sumv(products).x;
					result.row_at(row)[col] = sum;
				}
			}
			return result;
		}
	};
	
	// TODO: Optimized multiplier for 44, 43, 34, 33...
	
	template <typename ElementType, size_t N, size_t M, size_t P>
	TMatrix<ElementType, N, P>
	matrix_multiply(const TMatrix<ElementType, N, M>& a, const TMatrix<ElementType, P, N>& b) {
		return MatrixMultiplier<ElementType, N, M, P>::multiply(a, b);
	}
	
	template <typename ElementType, size_t N, size_t M>
	TVector<ElementType, M> matrix_transform(const TMatrix<ElementType, N, M>& matrix, TVector<ElementType, N> input) {
		TVector<ElementType, M> result;
		for (size_t row = 0; row < M; ++row) {
			result[row] = sumv(matrix.row_at(row) * input).x;
		}
		return result;
	}
}

#endif
