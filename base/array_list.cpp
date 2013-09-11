#include "base/array_list.hpp"
#include "base/exceptions.hpp"
#include "base/raise.hpp"

namespace grace {
	namespace detail {
		void array_list_index_out_of_bounds(size_t idx, size_t max) {
			raise<IndexOutOfBoundsException>("Requested index {0} in ArrayList of size {1}.", idx, max);
		}
	}
}