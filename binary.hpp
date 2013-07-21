//
//  binary_document.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_binary_document_hpp
#define grace_binary_document_hpp

#include "serialization/document.hpp"
#include "serialization/document_node.hpp"

#include "base/bag.hpp"

namespace grace {
	struct BinarySerializer : IDocumentReader, IDocumentWriter {
		size_t read(Document& doc, InputStream& is, String& out_error) final;
		void write(OutputStream& os, const Document& doc) final;
		bool can_parse(const byte* begin, const byte* end) const;
	private:
		void write_node(const DocumentNode&, OutputStream& os) const;
		bool read_node(DocumentNode&, const byte*& ptr, const byte* end, String& out_error);
	};
}

#endif
