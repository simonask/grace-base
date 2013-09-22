#pragma once
#ifndef ARCHIVE_NODE_HPP_EP8GSONT
#define ARCHIVE_NODE_HPP_EP8GSONT

#include "base/string.hpp"
#include "base/map.hpp"
#include "base/either.hpp"
#include "type/type.hpp"
#include "type/array_type.hpp"
#include "type/dictionary_type.hpp"
#include "base/dictionary.hpp"
#include "type/any_type.hpp"

namespace grace {

struct Document;
struct DeserializeReferenceBase;
struct SerializeReferenceBase;
struct DeserializeSignalBase;
struct IUniverse;
struct ISlot;
struct DerivedType;
class Object;

struct DocumentNode {
	bool is_empty() const;
	bool is_array() const;
	bool is_map() const;
	bool is_string() const;
	bool is_float() const;
	bool is_integer() const;
	bool is_scalar() const;
	
	// Basic deserializers
	
	bool operator>>(bool& out_bool) const;
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, bool>::type
	operator>>(T& out_integer) const;
	
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, bool>::type
	operator>>(T& out_float) const;
	
	bool operator>>(String& out_str) const;
	bool operator>>(StringRef& out_str) const;
	
	template <typename T>
	bool operator>>(Array<T>& array) const;
	template <typename T>
	bool operator>>(Dictionary<T>& dictionary) const;
	
	// Basic serializers
	
	void operator<<(bool in_bool);
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	operator<<(T in_integer);
	
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, void>::type
	operator<<(T in_float);
	
	void operator<<(const String& in_str);
	void operator<<(StringRef in_str);
	void operator<<(const char* in_str) { (*this) << StringRef(in_str); }
	void operator<<(const Any& any);
	
	template <typename T>
	void operator<<(const Array<T>& array);
	template <typename T>
	void operator<<(const Dictionary<T>& dictionary);
	
	void operator<<(NothingType);
	
	void clear() { (*this) << Nothing; }
	
	// Querying arrays/dictionaries directly
	const DocumentNode& operator[](size_t idx) const;
	const DocumentNode& operator[](StringRef key) const;
	template <typename F>
	void array_each(F f) const;
	template <typename F>
	void map_each_pair(F f) const;
	
	// Constructing arrays/dictionaries directly
	DocumentNode& operator[](size_t idx);
	DocumentNode& operator[](StringRef key);
	DocumentNode& array_push(); // in-place array construction
	size_t array_size() const;
	
	Document& document() const { return document_; }
	IAllocator& allocator() const;
	
	using MapType     = Dictionary<DocumentNode*>;
	using ArrayType   = Array<DocumentNode*>;
	using StringType  = String;
	using IntegerType = int64;
	using FloatType   = float64;
	
	void dump(FormattedStream& os) const;
	
	using InternalValueType = Either<NothingType, IntegerType, FloatType, StringType, ArrayType, MapType>;
	
	template <typename T, typename... Args>
	auto when(Args&&... args) -> decltype(InternalValueType(Nothing).when<T>(std::forward<Args>(args)...)) {
		return value_.when<T>(std::forward<Args>(args)...);
	}
	template <typename T, typename... Args>
	auto when(Args&&... args) const -> decltype(((const InternalValueType)InternalValueType(Nothing)).when<T>(std::forward<Args>(args)...)) {
		return value_.when<T>(std::forward<Args>(args)...);
	}
	
	template <typename F>
	void walk(F callback);
	template <typename F>
	void walk(F callback) const;
	
	InternalValueType& internal_value() { return value_; }
	const InternalValueType& internal_value() const { return value_; }
	
	explicit DocumentNode(Document& document) : document_(document) {}
protected:
	void dump(FormattedStream& os, int indent) const;
protected:
	friend struct Document;
	Document& document_;
	InternalValueType value_ = InternalValueType(Nothing);
	
	DocumentNode* make_child();
};

// This type is provided as a way to defer deserialization of serialized
// subtrees, for instance when loading scenes via serialized RPC calls.
// 'Deserialize' merely converts the reference to a pointer and assigns
// it to the place.
struct DocumentNodeConstPtrType : TypeFor<const DocumentNode*> {
	using T = const DocumentNode*;
	void deserialize(T& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const T& place, DocumentNode&, IUniverse&) const final;
	StringRef name() const final;
};
template <>
struct BuildTypeInfo<const DocumentNode*> {
	static const DocumentNodeConstPtrType* build();
};
struct DocumentNodePtrType : TypeFor<DocumentNode*> {
	using T = DocumentNode*;
	void deserialize(T& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const T& place, DocumentNode&, IUniverse&) const final;
	StringRef name() const final;
};
template <>
struct BuildTypeInfo<DocumentNode*> {
	static const DocumentNodePtrType* build();
};

	inline bool DocumentNode::operator>>(bool& out_bool) const {
		StringRef str;
		if (*this >> str) {
			out_bool = str != "false";
			return true;
		}
		return false;
	}
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, bool>::type
	DocumentNode::operator>>(T& out_integer) const {
		bool result = false;
		value_.when<IntegerType>([&](IntegerType n) {
			// TODO: Warn if integer is out of range of T.
			out_integer = (T)n;
			result = true;
		});
		value_.when<FloatType>([&](FloatType f) {
			out_integer = (T)f;
			result = true;
		});
		return result;
	}
	
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, bool>::type
	DocumentNode::operator>>(T& out_float) const {
		bool result = false;
		value_.when<FloatType>([&](FloatType f) {
			out_float = f;
			result = true;
		});
		value_.when<IntegerType>([&](IntegerType n) {
			out_float = n;
			result = true;
		});
		return result;
	}
	
	inline bool DocumentNode::operator>>(String& out_str) const {
		bool result = false;
		value_.when<StringType>([&](const StringType& str) {
			out_str = str;
			result = true;
		});
		return result;
	}
	
	inline bool DocumentNode::operator>>(StringRef& out_str) const {
		bool result = false;
		value_.when<StringType>([&](const StringType& str) {
			out_str = str;
			result = true;
		});
		return result;
	}
	
	template <typename T>
	bool DocumentNode::operator>>(Array<T>& array) const {
		bool result = false;
		value_.when<ArrayType>([&](const ArrayType& arr) {
			result = true;
			array.reserve(arr.size());
			for (const DocumentNode* node: arr) {
				T element;
				result = ((*node) >> element) && result;
				array.push_back(move(element));
			}
		});
		return result;
	}
	
	template <typename T>
	bool DocumentNode::operator>>(Dictionary<T>& dictionary) const {
		bool result = false;
		value_.when<MapType>([&](const MapType& dict) {
			result = true;
			//dictionary.reserve(dict.size());
			for (auto pair: dict) {
				T element;
				result = ((*pair.second) >> element) && result;
				dictionary[pair.first] = move(element);
			}
		});
		return result;
	}
	
	inline void DocumentNode::operator<<(bool in_bool) {
		value_ = StringType(in_bool ? "true" : "false");
	}
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	DocumentNode::operator<<(T in_integer) {
		IntegerType n = (IntegerType)in_integer; // XXX: Potential information loss
		value_ = n;
	}
	
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, void>::type
	DocumentNode::operator<<(T in_float) {
		FloatType f = (FloatType)in_float; // XXX: Potential information loss
		value_ = f;
	}
	
	inline void DocumentNode::operator<<(const String& in_str) {
		value_ = StringType(in_str, allocator());
	}
	
	inline void DocumentNode::operator<<(StringRef in_str) {
		value_ = StringType(in_str, allocator());
	}
	
	inline void DocumentNode::operator<<(const Any& in_any) {
		if (in_any.is_empty()) {
			clear();
		} else {
			in_any.type()->serialize_raw(in_any.ptr(), *this, *(IUniverse*)nullptr);
		}
	}
	
	template <typename T>
	void DocumentNode::operator<<(const Array<T>& array) {
		ArrayType arr(allocator());
		arr.reserve(array.size());
		for (auto& x: array) {
			DocumentNode* node = make_child();
			(*node) << x;
			arr.push_back(node);
		}
		value_ = move(arr);
	}
	
	template <typename T>
	void DocumentNode::operator<<(const Dictionary<T>& dictionary) {
		MapType m(allocator());
		//m.reserve(dictionary.size());
		for (auto pair: dictionary) {
			DocumentNode* node = make_child();
			(*node) << pair.second;
			m[pair.first] = node;
		}
		value_ = move(m);
	}
	
	inline void DocumentNode::operator<<(NothingType) {
		value_ = Nothing;
	}

	template <typename F>
	void DocumentNode::map_each_pair(F f) const {
		value_.when<MapType>([&](const MapType& map) {
			for (auto pair: map) {
				f(pair.first, pair.second);
			}
		});
	}
	
	template <typename F>
	void DocumentNode::array_each(F f) const {
		value_.when<ArrayType>([&](const ArrayType& array) {
			for (auto& element: array) {
				f(*element);
			}
		});
	}
	
	template <typename F>
	void DocumentNode::walk(F callback) {
		value_.visit(callback);
	}
	
	template <typename F>
	void DocumentNode::walk(F callback) const {
		value_.visit(callback);
	}
	
	void dump_document_node_to_stdout(const DocumentNode& node);
	void dump_document_to_stdout(const Document& document);
}

#endif /* end of include guard: ARCHIVE_NODE_HPP_EP8GSONT */
