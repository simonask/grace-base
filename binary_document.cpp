//
//  binary_document.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "serialization/binary_document.hpp"
#include "base/log.hpp"
#include "io/util.hpp"

namespace grace {
	namespace {
		enum class NodeType : uint8 {
			Empty,
			Array,
			Map,
			Integer,
			Float,
			String,
		};
	
		void write_byte(OutputStream& os, byte b) {
			os.write(&b, 1);
		}
		
		template <typename T>
		void write_bytes(OutputStream& os, const T* value) {
			static_assert(std::is_pod<T>::value, "Cannot write bytes of non-POD object to stream.");
			auto ptr = reinterpret_cast<const byte*>(value);
			for (auto p = ptr; p < ptr+sizeof(T); ++p) {
				write_byte(os, *p);
			}
		}
		
		void write_bytes(OutputStream& os, const char* begin, size_t len) {
			for (auto p = begin; p < begin+len; ++p) {
				write_byte(os, *p);
			}
		}
		
		template <typename T>
		bool read_bytes(const byte*& p, const byte* end, T* value) {
			static_assert(std::is_pod<T>::value, "Cannot read bytes of non-POD object from stream.");
			if (end - p < sizeof(T)) return false;
			byte* dst = reinterpret_cast<byte*>(value);
			for (size_t i = 0; i < sizeof(T); ++i) {
				dst[i] = p[i];
			}
			p += sizeof(T);
			return true;
		}
	}
	
	void BinarySerializer::write_node(const DocumentNode& n, OutputStream& os) const {
		n.when<DocumentNode::StringType>([&](const DocumentNode::StringType& str) {
			write_byte(os, (byte)NodeType::String);
			uint32 string_length = (uint32)str.size();
			write_bytes(os, &string_length);
			write_bytes(os, str.data(), string_length);
		}).when<DocumentNode::ArrayType>([&](const DocumentNode::ArrayType& arr) {
			write_byte(os, (byte)NodeType::Array);
			uint32 array_length = arr.size();
			write_bytes(os, &array_length);
			for (auto it: arr) {
				write_node(*it, os);
			}
		}).when<DocumentNode::MapType>([&](const DocumentNode::MapType& map) {
			write_byte(os, (byte)NodeType::Map);
			uint32 map_length = (uint32)map.size();
			write_bytes(os, &map_length);
			for (auto it: map) {
				uint32_t string_length = (uint32)it.first.size();
				write_bytes(os, &string_length);
				write_bytes(os, it.first.data(), string_length);
				write_node(*it.second, os);
			}
		}).when<DocumentNode::IntegerType>([&](DocumentNode::IntegerType n) {
			write_byte(os, (byte)NodeType::Integer);
			write_bytes(os, &n);
		}).when<DocumentNode::FloatType>([&](DocumentNode::FloatType f) {
			write_byte(os, (byte)NodeType::Float);
			write_bytes(os, &f);
		}).otherwise([&]() {
			ASSERT(n.is_empty()); // Invalid node type!
			write_byte(os, (byte)NodeType::Empty);
		});
	}
	
	bool BinarySerializer::read_node(DocumentNode& n, const byte*& p, const byte *end, String &out_error) {
		byte t;
		if (!read_bytes(p, end, &t)) {
			out_error = "Unexpected end of stream.";
			return false;
		}
		NodeType type = (NodeType)t;
		
		switch (type) {
			case NodeType::Empty: { n.clear(); return true; }
			case NodeType::String: {
				uint32 string_length;
				if (!read_bytes(p, end, &string_length)) {
					out_error = "Invalid string length (corrupt stream).";
					return false;
				}
				if (p + string_length > end) {
					out_error = "Unexpected end of stream (corrupt string length).";
					return false;
				}
				StringRef str(reinterpret_cast<const char*>(p), string_length);
				n << str;
				p += string_length;
				return true;
			}
			case NodeType::Array: {
				uint32 array_length;
				if (!read_bytes(p, end, &array_length)) {
					out_error = "Invalid array length (corrupt stream).";
					return false;
				}
				DocumentNode::ArrayType tmp(n.allocator());
				tmp.reserve(array_length);
				n.internal_value() = move(tmp);
				for (uint32 i = 0; i < array_length; ++i) {
					DocumentNode& node = n.array_push();
					if (!read_node(node, p, end, out_error)) {
						return false;
					}
				}
				return true;
			}
			case NodeType::Map: {
				uint32 map_length;
				if (!read_bytes(p, end, &map_length)) {
					out_error = "Invalid map length (corrupt stream).";
					return false;
				}
				for (uint32 i = 0; i < map_length; ++i) {
					uint32 string_length;
					if (!read_bytes(p, end, &string_length)) {
						out_error = "Invalid map key length (corrupt stream).";
						return false;
					}
					StringRef key = StringRef(reinterpret_cast<const char*>(p), string_length);
					p += string_length;
					DocumentNode& value = n[key];
					if (!read_node(value, p, end, out_error)) {
						return false;
					}
				}
				return true;
			}
			case NodeType::Integer: {
				DocumentNode::IntegerType integer_value;
				if (!read_bytes(p, end, &integer_value)) {
					out_error = "Invalid integer value (corrupt stream).";
					return false;
				}
				n << integer_value;
				return true;
			}
			case NodeType::Float: {
				DocumentNode::FloatType float_value;
				if (!read_bytes(p, end, &float_value)) {
					out_error = "Invalid float value (corrupt stream).";
					return false;
				}
				n << float_value;
				return true;
			}
			default:
				out_error = "Invalid type (corrupt stream).";
				return false;
		}
	}
	
	void BinarySerializer::write(OutputStream &os, const Document& doc) {
		StringStream ss;
		write_node(doc, ss);
		String data = ss.str();
		uint32 stream_length = (uint32)data.size();
		write_bytes(os, &stream_length);
		FormattedStream(os) << data;
	}
	
	size_t BinarySerializer::read(Document& doc, InputStream& is, grace::String& out_error) {
		doc.clear();
		if (is.has_length()) {
			size_t stream_length = is.length();
			Array<byte> buffer = read_all<Array<byte>>(is);
			const byte* p = buffer.data();
			const byte* end = p + stream_length;
			uint32 data_length;
			if (!read_bytes(p, end, &data_length)) {
				out_error = "Wrong data length, or not all data is available yet.";
				return 0;
			}
			
			if (!read_node(doc, p, end, out_error)) {
				doc.clear();
				return 0;
			}
			
			return p - buffer.data();
		}
		return 0;
	}
	
	bool BinarySerializer::can_parse(const byte* begin, const byte* end) const {
		uint32 stream_length;
		const byte* p = begin;
		if (!read_bytes(p, end, &stream_length)) {
			return false;
		}
		return (end - p) >= stream_length;
	}
}
