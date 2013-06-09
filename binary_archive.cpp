//
//  binary_archive.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "serialization/binary_archive.hpp"
#include "base/log.hpp"
#include "io/util.hpp"

namespace grace {
	namespace {
		enum NodeType : uint8 {
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
	
	void BinaryArchiveNode::write(OutputStream& os) const {
		NodeType t;
		if (is_empty()) {
			t = NodeType::Empty;
		} else if (is_string()) {
			t = NodeType::String;
		} else if (is_array()) {
			t = NodeType::Array;
		} else if (is_map()) {
			t = NodeType::Map;
		} else if (is_integer()) {
			t = NodeType::Integer;
		} else if (is_float()) {
			t = NodeType::Float;
		} else {
			ASSERT(false); // Invalid node!
		}
		
		ASSERT(t <= UINT8_MAX);
		write_byte(os, (byte)t);
		switch (t) {
			case NodeType::Empty: { break; }
			case NodeType::String: {
				value_.when<StringType>([&](const StringType& string_value) {
					uint32 string_length = (uint32)string_value.size();
					write_bytes(os, &string_length);
					write_bytes(os, string_value.data(), string_length);
				});
				break;
			}
			case NodeType::Array: {
				value_.when<ArrayType>([&](const ArrayType& array) {
					uint32 array_length = array.size();
					write_bytes(os, &array_length);
					for (auto it: array) {
						((BinaryArchiveNode*)it)->write(os);
					}
				});
				break;
			}
			case NodeType::Map: {
				value_.when<MapType>([&](const MapType& map) {
					uint32 map_length = (uint32)map.size();
					write_bytes(os, &map_length);
					for (auto it: map) {
						uint32_t string_length = (uint32)it.first.size();
						write_bytes(os, &string_length);
						write_bytes(os, it.first.data(), string_length);
						((BinaryArchiveNode*)it.second)->write(os);
					}
				});
				break;
			}
			case NodeType::Integer: {
				value_.when<IntegerType>([&](const IntegerType& n) {
					write_bytes(os, &n);
				});
				break;
			}
			case NodeType::Float: {
				value_.when<FloatType>([&](const FloatType& f) {
					write_bytes(os, &f);
				});
				break;
			}
		}
	}
	
	bool BinaryArchiveNode::read(const byte*& p, const byte *end, grace::String &out_error) {
		byte type;
		if (!read_bytes(p, end, &type)) {
			out_error = "Unexpected end of stream.";
			return false;
		}
		
		switch (type) {
			case NodeType::Empty: { clear(); return true; }
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
				value_ = StringType(reinterpret_cast<const char*>(p), string_length, allocator());
				p += string_length;
				return true;
			}
			case NodeType::Array: {
				uint32 array_length;
				if (!read_bytes(p, end, &array_length)) {
					out_error = "Invalid array length (corrupt stream).";
					return false;
				}
				ArrayType tmp(allocator());
				tmp.reserve(array_length);
				value_ = move(tmp);
				for (uint32 i = 0; i < array_length; ++i) {
					BinaryArchiveNode& node = static_cast<BinaryArchiveNode&>(array_push());
					if (!node.read(p, end, out_error)) {
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
					BinaryArchiveNode& value = static_cast<BinaryArchiveNode&>((*this)[key]);
					if (!value.read(p, end, out_error)) {
						return false;
					}
				}
				return true;
			}
			case NodeType::Integer: {
				IntegerType integer_value;
				if (!read_bytes(p, end, &integer_value)) {
					out_error = "Invalid integer value (corrupt stream).";
					return false;
				}
				*this << integer_value;
				return true;
			}
			case NodeType::Float: {
				FloatType float_value;
				if (!read_bytes(p, end, &float_value)) {
					out_error = "Invalid float value (corrupt stream).";
					return false;
				}
				*this << float_value;
				return true;
			}
			default:
				out_error = "Invalid type (corrupt stream).";
				return false;
		}
	}
	
	BinaryArchive::BinaryArchive(IAllocator& alloc) : Archive(alloc), root_(*this), empty_(*this), nodes_(alloc) {
		clear();
	}
	
	void BinaryArchive::write(OutputStream &os) const {
		StringStream ss;
		root_.write(ss);
		grace::String data = ss.str();
		uint32 stream_length = (uint32)data.size();
		write_bytes(os, &stream_length);
		FormattedStream(os) << data;
	}
	
	size_t BinaryArchive::read(InputStream& is, grace::String& out_error) {
		clear();
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
			
			if (!root_.read(p, end, out_error)) {
				clear();
				return 0;
			}
			
			return p - buffer.data();
		}
		return 0;
	}
	
	bool BinaryArchive::can_parse(const byte* begin, const byte* end) const {
		uint32 stream_length;
		const byte* p = begin;
		if (!read_bytes(p, end, &stream_length)) {
			return false;
		}
		return (end - p) >= stream_length;
	}
	
	void BinaryArchive::clear() {
		nodes_.clear();
		root_.clear();
	}
	
	BinaryArchiveNode* BinaryArchive::make_internal() {
		BinaryArchiveNode* node = nodes_.allocate(*this);
		return node;
	}
	
	
}
