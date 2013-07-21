//
//  yaml_document.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_yaml_document_hpp
#define grace_yaml_document_hpp

#include "serialization/document.hpp"

namespace grace {
	struct YAMLDocument : public Document {
		explicit YAMLDocument(IAllocator& alloc = default_allocator()) : Document(alloc) {}
		void write(OutputStream& os) const;
		size_t read(InputStream& is, String& out_error);
		bool can_parse(const byte* begin, const byte* end) const;
	};
}

#endif
