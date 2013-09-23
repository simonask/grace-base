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
	struct YAML : IDocumentReader, IDocumentWriter {
		void write(IOutputStream& os, const Document& doc) final;
		size_t read(Document& doc, IInputStream& is, String& out_error) final;
		bool can_parse(const byte* begin, const byte* end) const;
	};
}

#endif
