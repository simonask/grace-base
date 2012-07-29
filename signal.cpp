#include "object/signal.hpp"
#include "object/object_type.hpp"
#include "base/log.hpp"
#include "io/string_stream.hpp"

namespace falling {

std::string SignalTypeBase::build_signal_name(const Array<const Type*>& signature) {
	StringStream ss;
	ss << "Signal<";
	for (size_t i = 0; i < signature.size(); ++i) {
		ss << signature[i]->name();
		if (i+1 != signature.size()) {
			ss << ", ";
		}
	}
	ss << ">";
	return ss.str();
}
	
	void SignalTypeBase::report_invalid_signal_connection_warning() {
		Warning() << "Invalid signal connection.";
	}

	void SignalTypeBase::report_non_map_signal_connection_warning() {
		Warning() << "Non-map signal connection node. Did you forget to write a scene upgrader?";
	}
}