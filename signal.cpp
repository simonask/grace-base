#include "object/signal.hpp"
#include "object/signal_type.hpp"
#include "object/object_type.hpp"
#include "base/log.hpp"
#include "io/string_stream.hpp"

namespace grace {

String SignalTypeBase::build_signal_name(ArrayRef<const Type*> signature) {
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
	
	void nonexistent_slot_warning(ObjectPtr<> receiver, StringRef slot_name) {
		Warning() << "Object " << receiver->object_id() << " dot not have a slot named " << slot_name << ".";
	}
	
	void slot_type_mismatch_warning(ObjectPtr<> receiver, StringRef slot_name, String expected_signature_description, String signature_description) {
		Warning() << "Tried to connect signal to slot '" << slot_name << "', which has a different signature. Expected " << expected_signature_description << ", got " << signature_description << ".";
	}
	
}