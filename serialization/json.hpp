#pragma once
#ifndef JSON_ARCHIVE_HPP_4OX35IUJ
#define JSON_ARCHIVE_HPP_4OX35IUJ

#include "serialization/document.hpp"
#include "serialization/document_node.hpp"
#include "base/bag.hpp"
#include "base/map.hpp"
#include "base/string.hpp"

namespace grace {
	struct JSON : IDocumentReader, IDocumentWriter {
		size_t read(Document& doc, IInputStream& is, String& out_error) final;
		void write(IOutputStream& os, const Document& doc) final;
	private:
		void write_node(const DocumentNode&, IOutputStream& os, bool print_inline, int indent) const;
		bool read_node(DocumentNode&, const byte*& p, const byte* end, String& out_error);
	};
}

#endif /* end of include guard: JSON_ARCHIVE_HPP_4OX35IUJ */
