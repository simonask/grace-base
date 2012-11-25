//
//  binary_archive.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "serialization/binary_archive.hpp"
#include "base/log.hpp"
#include "io/util.hpp"

namespace falling {
	namespace {
		void write_byte(OutputStream& os, byte b) {
			os.write(&b, 1);
		}
		
		template <typename T>
		void write_bytes(OutputStream& os, const T* value) {
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
		ArchiveNodeType::Type t = type();
		ASSERT(t <= UINT8_MAX);
		write_byte(os, (byte)t);
		switch (t) {
			case ArchiveNodeType::Empty: { break; }
			case ArchiveNodeType::String: {
				uint32 string_length = (uint32)string_value.size();
				write_bytes(os, &string_length);
				write_bytes(os, string_value.c_str(), string_length);
				break;
			}
			case ArchiveNodeType::Array: {
				uint32 array_length = array_.size();
				write_bytes(os, &array_length);
				for (auto it: array_) {
					((BinaryArchiveNode*)it)->write(os);
				}
				break;
			}
			case ArchiveNodeType::Map: {
				uint32 map_length = (uint32)map_.size();
				write_bytes(os, &map_length);
				for (auto& it: map_) {
					uint32_t string_length = (uint32)it.first.size();
					write_bytes(os, &string_length);
					write_bytes(os, it.first.c_str(), string_length);
					((BinaryArchiveNode*)it.second)->write(os);
				}
				break;
			}
			case ArchiveNodeType::Integer: {
				write_bytes(os, &integer_value);
				break;
			}
			case ArchiveNodeType::Float: {
				write_bytes(os, &float_value);
				break;
			}
		}
	}
	
	bool BinaryArchiveNode::read(const byte*& p, const byte *end, std::string &out_error) {
		byte type;
		if (!read_bytes(p, end, &type)) {
			out_error = "Unexpected end of stream.";
			return false;
		}
		
		switch (type) {
			case ArchiveNodeType::Empty: { clear(); return true; }
			case ArchiveNodeType::String: {
				clear(ArchiveNodeType::String);
				uint32 string_length;
				if (!read_bytes(p, end, &string_length)) {
					out_error = "Invalid string length (corrupt stream).";
					return false;
				}
				if (p + string_length > end) {
					out_error = "Unexpected end of stream (corrupt string length).";
					return false;
				}
				string_value = std::string(reinterpret_cast<const char*>(p), string_length);
				p += string_length;
				return true;
			}
			case ArchiveNodeType::Array: {
				clear(ArchiveNodeType::Array);
				uint32 array_length;
				if (!read_bytes(p, end, &array_length)) {
					out_error = "Invalid array length (corrupt stream).";
					return false;
				}
				array_.reserve(array_length);
				for (uint32 i = 0; i < array_length; ++i) {
					BinaryArchiveNode& node = static_cast<BinaryArchiveNode&>(array_push());
					if (!node.read(p, end, out_error)) {
						return false;
					}
				}
				return true;
			}
			case ArchiveNodeType::Map: {
				clear(ArchiveNodeType::Map);
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
					std::string key = std::string(reinterpret_cast<const char*>(p), string_length);
					p += string_length;
					BinaryArchiveNode& value = static_cast<BinaryArchiveNode&>((*this)[key]);
					if (!value.read(p, end, out_error)) {
						return false;
					}
				}
				return true;
			}
			case ArchiveNodeType::Integer: {
				clear(ArchiveNodeType::Integer);
				if (!read_bytes(p, end, &integer_value)) {
					out_error = "Invalid integer value (corrupt stream).";
					return false;
				}
				return true;
			}
			case ArchiveNodeType::Float: {
				clear(ArchiveNodeType::Float);
				if (!read_bytes(p, end, &float_value)) {
					out_error = "Invalid float value (corrupt stream).";
					return false;
				}
				return true;
			}
			default:
				out_error = "Invalid type (corrupt stream).";
				return false;
		}
	}
	
	BinaryArchive::BinaryArchive(IAllocator& alloc) : Archive(alloc), root_(*this, ArchiveNodeType::Map), empty_(*this, ArchiveNodeType::Empty), nodes_(alloc) {
		clear();
	}
	
	void BinaryArchive::write(OutputStream &os) const {
		StringStream ss;
		root_.write(ss);
		std::string data = ss.str();
		uint32 stream_length = (uint32)data.size();
		write_bytes(os, &stream_length);
		FormattedStream(os) << data;
	}
	
	size_t BinaryArchive::read(InputStream& is, std::string& out_error) {
		clear();
		if (is.has_length()) {
			size_t stream_length = is.length();
			Array<byte> buffer = read_all(is);
			const byte* p = buffer.data();
			const byte* end = p + stream_length;
			if (!read_bytes(p, end, &stream_length)) {
				out_error = "Wrong stream length, or not all data is available yet.";
				return 0;
			}
			
			if (!static_cast<BinaryArchiveNode&>(root()).read(p, end, out_error)) {
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
	
	BinaryArchiveNode* BinaryArchive::make_internal(ArchiveNodeType::Type t) {
		BinaryArchiveNode* node = nodes_.allocate(*this, t);
		return node;
	}
	
	
}
