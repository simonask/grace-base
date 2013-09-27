#include "object/signal.hpp"
#include "object/signal_type.hpp"
#include "object/object_type.hpp"
#include "base/log.hpp"
#include "base/string.hpp"

namespace grace {

String SignalTypeBase::build_signal_name(ArrayRef<const IType*> signature) {
	Array<StringRef> names;
	names.resize(signature.size());
	std::transform(signature.begin(), signature.end(), names.begin(), std::bind(&IType::name, std::placeholders::_1));
	return encapsulate_join(names.ref(), ", ", "Signal<", ">");
}
	
	void nonexistent_slot_warning(ObjectPtr<> receiver, StringRef slot_name) {
		Warning() << "Object " << receiver->object_id() << " dot not have a slot named " << slot_name << ".";
	}
	
	void slot_type_mismatch_warning(ObjectPtr<> receiver, StringRef slot_name, String expected_signature_description, String signature_description) {
		Warning() << "Tried to connect signal to slot '" << slot_name << "', which has a different signature. Expected " << expected_signature_description << ", got " << signature_description << ".";
	}
	
}