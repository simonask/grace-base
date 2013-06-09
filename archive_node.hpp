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

struct Archive;
struct DeserializeReferenceBase;
struct SerializeReferenceBase;
struct DeserializeSignalBase;
struct IUniverse;
struct ISlot;
struct DerivedType;
struct Object;

struct ArchiveNode {
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
	const ArchiveNode& operator[](size_t idx) const;
	const ArchiveNode& operator[](StringRef key) const;
	template <typename F>
	void array_each(F f) const;
	template <typename F>
	void map_each_pair(F f) const;
	
	// Constructing arrays/dictionaries directly
	ArchiveNode& operator[](size_t idx);
	ArchiveNode& operator[](StringRef key);
	ArchiveNode& array_push(); // in-place array construction
	size_t array_size() const;
	
	
	virtual ~ArchiveNode() {}
	Archive& archive() const { return archive_; }
	IAllocator& allocator() const;
	
	using MapType     = Dictionary<ArchiveNode*>;
	using ArrayType   = Array<ArchiveNode*>;
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
	
	InternalValueType& internal_value() { return value_; }
	const InternalValueType& internal_value() const { return value_; }
protected:
	explicit ArchiveNode(Archive& archive) : archive_(archive) {}
	
	void dump(FormattedStream& os, int indent) const;
protected:
	Archive& archive_;
	InternalValueType value_ = InternalValueType(Nothing);
	
	ArchiveNode* make_child();
};

// This type is provided as a way to defer deserialization of serialized
// subtrees, for instance when loading scenes via serialized RPC calls.
// 'Deserialize' merely converts the reference to a pointer and assigns
// it to the place.
struct ArchiveNodeConstPtrType : TypeFor<const ArchiveNode*> {
	using T = const ArchiveNode*;
	void deserialize(T& place, const ArchiveNode&, IUniverse&) const final;
	void serialize(const T& place, ArchiveNode&, IUniverse&) const final;
	StringRef name() const final;
};
template <>
struct BuildTypeInfo<const ArchiveNode*> {
	static const ArchiveNodeConstPtrType* build();
};
struct ArchiveNodePtrType : TypeFor<ArchiveNode*> {
	using T = ArchiveNode*;
	void deserialize(T& place, const ArchiveNode&, IUniverse&) const final;
	void serialize(const T& place, ArchiveNode&, IUniverse&) const final;
	StringRef name() const final;
};
template <>
struct BuildTypeInfo<ArchiveNode*> {
	static const ArchiveNodePtrType* build();
};

	inline bool ArchiveNode::operator>>(bool& out_bool) const {
		StringRef str;
		if (*this >> str) {
			out_bool = str != "false";
			return true;
		}
		return false;
	}
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, bool>::type
	ArchiveNode::operator>>(T& out_integer) const {
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
	ArchiveNode::operator>>(T& out_float) const {
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
	
	inline bool ArchiveNode::operator>>(String& out_str) const {
		bool result = false;
		value_.when<StringType>([&](const StringType& str) {
			out_str = str;
			result = true;
		});
		return result;
	}
	
	inline bool ArchiveNode::operator>>(StringRef& out_str) const {
		bool result = false;
		value_.when<StringType>([&](const StringType& str) {
			out_str = str;
			result = true;
		});
		return result;
	}
	
	template <typename T>
	bool ArchiveNode::operator>>(Array<T>& array) const {
		bool result = false;
		value_.when<ArrayType>([&](const ArrayType& arr) {
			result = true;
			array.reserve(arr.size());
			for (const ArchiveNode* node: arr) {
				T element;
				result = ((*node) >> element) && result;
				array.push_back(move(element));
			}
		});
		return result;
	}
	
	template <typename T>
	bool ArchiveNode::operator>>(Dictionary<T>& dictionary) const {
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
	
	inline void ArchiveNode::operator<<(bool in_bool) {
		value_ = StringType(in_bool ? "true" : "false");
	}
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	ArchiveNode::operator<<(T in_integer) {
		IntegerType n = (IntegerType)in_integer; // XXX: Potential information loss
		value_ = n;
	}
	
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, void>::type
	ArchiveNode::operator<<(T in_float) {
		FloatType f = (FloatType)in_float; // XXX: Potential information loss
		value_ = f;
	}
	
	inline void ArchiveNode::operator<<(const String& in_str) {
		value_ = StringType(in_str, allocator());
	}
	
	inline void ArchiveNode::operator<<(StringRef in_str) {
		value_ = StringType(in_str, allocator());
	}
	
	inline void ArchiveNode::operator<<(const Any& in_any) {
		if (in_any.is_empty()) {
			clear();
		} else {
			in_any.type()->serialize_raw(in_any.ptr(), *this, *(IUniverse*)nullptr);
		}
	}
	
	template <typename T>
	void ArchiveNode::operator<<(const Array<T>& array) {
		ArrayType arr(allocator());
		arr.reserve(array.size());
		for (auto& x: array) {
			ArchiveNode* node = make_child();
			(*node) << x;
			arr.push_back(node);
		}
		value_ = move(arr);
	}
	
	template <typename T>
	void ArchiveNode::operator<<(const Dictionary<T>& dictionary) {
		MapType m(allocator());
		//m.reserve(dictionary.size());
		for (auto pair: dictionary) {
			ArchiveNode* node = make_child();
			(*node) << pair.second;
			m[pair.first] = node;
		}
		value_ = move(m);
	}
	
	inline void ArchiveNode::operator<<(NothingType) {
		value_ = Nothing;
	}

	template <typename F>
	void ArchiveNode::map_each_pair(F f) const {
		value_.when<MapType>([&](const MapType& map) {
			for (auto pair: map) {
				f(pair.first, pair.second);
			}
		});
	}
	
	template <typename F>
	void ArchiveNode::array_each(F f) const {
		value_.when<ArrayType>([&](const ArrayType& array) {
			for (auto& element: array) {
				f(*element);
			}
		});
	}
	
	void dump_archive_node_to_stdout(const ArchiveNode& node);
	void dump_archive_to_stdout(const Archive& archive);
}

#endif /* end of include guard: ARCHIVE_NODE_HPP_EP8GSONT */
