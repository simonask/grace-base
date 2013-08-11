//
//  rect.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_rect_hpp
#define grace_rect_hpp

#include "base/basic.hpp"
#include "base/maybe.hpp"
#include "geometry/vector.hpp"
#include "type/type.hpp"

namespace grace {
	template <typename T>
	struct TRect {
		using V = TVector<T, 2>;
		using Self = TRect<T>;
		TRect() {}
		TRect(T x, T y, T w, T h) : origin(x, y), size(w, h) {}
		TRect(V origin, V size) : origin(origin), size(size) {}
		TRect(const Self& other) = default;
		Self& operator=(const Self&) = default;
		
		static TRect<T> zero() {
			return TRect<T>(V::zero(), V::zero());
		}
		
		V origin;
		V size;
		
		V intersection_area(Self other) const {
			auto self_end = origin + size;
			auto other_end = other.origin + other.size;
			auto end_xs = shuffle2<0, X, 1, X>(self_end, other_end);
			auto end_ys = shuffle2<0, Y, 1, Y>(self_end, other_end);
			auto origin_xs = shuffle2<0, X, 1, X>(origin, other.origin);
			auto origin_ys = shuffle2<0, Y, 1, Y>(origin, other.origin);
			auto overlap = max(V::zero(), min(end_xs, end_ys) - max(origin_xs, origin_ys));
			return overlap;
		}
		
		Self intersection(Self other) const {
			V isize = intersection_area(other);
			V iorigin = max(origin, other.origin);
			// Return 0-rect if no intersection
			iorigin = select(isize == approximately(V::zero(), 0), V::zero(), iorigin);
			return Self(iorigin, size);
		}
		
		bool intersects(V coordinate) const {
			auto end = origin + size;
			auto m0 = coordinate >= origin;
			auto m1 = coordinate < end;
			auto bools = (m0 & m1) & V::MaskVector::replicate(1);
			return bools.sum() == 2;
		}
		
		Maybe<V> intersection_line(V p0, V p1) const;
		
		bool intersects(const Self& other) const {
			return intersection_area(other) != approximately(V::zero(), 0.00001f);
		}
		
		Self join(Self other) const {
			return *this + other;
		}
		
		Self operator+(Self other) const {
			Self s = *this;
			s += other;
			return s;
		}
		Self& operator+=(Self other) {
			auto other_end = other.origin + other.size;
			auto self_end = origin + size;
			auto biggest_end = max(self_end, other_end);
			origin = min(origin, other.origin);
			size = biggest_end - origin;
			return *this;
		}
		
		Self operator+(V point) const {
			Self s = *this;
			s += point;
			return s;
		}
		Self& operator+=(V point) {
			auto current_max = origin + size;
			origin = min(origin, point);
			auto end = max(current_max, point);
			size = end - origin;
			return *this;
		}
		
		float32 area() const {
			return size.x * size.y;
		}
	};
	
	using Rect = TRect<float32>;
	
	struct IType;
	namespace detail {
		const IType* build_rect_type();
	}
	
	template <>
	struct BuildTypeInfo<Rect> {
		static const IType* build() { return detail::build_rect_type(); }
	};
	
	template <typename T>
	Maybe<typename TRect<T>::V>
	TRect<T>::intersection_line(V p0, V p1) const {
		V corners[4] = {
			origin,
			origin + V{size.x, 0},
			origin + V{0, size.y},
			origin + size,
		};
		for (size_t i = 0; i < 4; ++i) {
			vec2 q0 = corners[i];
			vec2 q1 = corners[(i+1) % 4];
			auto intersection = line_intersection(p0, p1, q0, q1);
			if (intersection.is_set()) {
				return intersection;
			}
		}
		return Nothing;
	}
}

#endif
