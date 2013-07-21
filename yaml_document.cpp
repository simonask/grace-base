//
//  yaml_document.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "serialization/yaml_document.hpp"
#include "io/util.hpp"
#include "base/parse.hpp"
#include "base/pair.hpp"

#include <yaml.h>
#include <stdexcept>

namespace grace {
	namespace {
		struct YAMLParserError : IException {
			String message_;
			YAMLParserError(StringRef message) : message_(message, default_allocator()) {}
			StringRef what() const final { return message_; }
		};
		
		struct YAMLParserState {
			YAMLDocument& document;
			enum StateType {
				TopLevel,
				MappingExpectingKey,
				MappingExpectingValue,
				Sequence,
			};
			
			ArrayList<Pair<DocumentNode*, String>> stack; // node and non-empty string if node is a mapping waiting for a value
			Array<DocumentNode*> roots;
			Map<String, DocumentNode*> anchors;
			YAMLParserState(YAMLDocument& document) : document(document) {}
			
			DocumentNode* root() const { return roots.size() ? roots[0] : nullptr; }
			DocumentNode* top() const { return stack.back().first; }
			StringRef top_key() const { return stack.back().second; }
			String& top_key() { return stack.back().second; }
			
			StateType state() const {
				if (stack.size() == 0) return TopLevel;
				if (top()->is_array()) return Sequence;
				if (top()->is_map()) {
					return top_key().size() == 0 ? MappingExpectingKey : MappingExpectingValue;
				}
				return TopLevel;
			}
			
			DocumentNode* make() {
				return document.make();
			}
			
			void push(DocumentNode* node) {
				if (node->is_array() || node->is_map()) {
					stack.push_back(Pair<DocumentNode*,String>{node, String("")});
				} else {
					throw YAMLParserError("Invalid node type for parser stack.");
				}
			}
			
			void add_value_to_top(DocumentNode* node) {
				switch (state()) {
					case TopLevel:
						roots.push_back(node);
						break;
					case Sequence:
						if (!top()->is_array()) {
							top()->internal_value() = DocumentNode::ArrayType(top()->allocator());
						}
						top()->when<DocumentNode::ArrayType>([&](DocumentNode::ArrayType& array) {
							array.push_back(node);
						});
						break;
					case MappingExpectingKey: {
						if (!node->is_string()) {
							throw YAMLParserError("Expected map key, but didn't get a string.");
						}
						*node >> top_key();
						break;
					}
					case MappingExpectingValue: {
						// Insert value in top mapping.
						if (!top()->is_map()) {
							top()->internal_value() = DocumentNode::MapType(top()->allocator());
						}
						top()->when<DocumentNode::MapType>([&](DocumentNode::MapType& map) {
							map[top_key()] = node;
						});
						top_key() = "";
						break;
					}
				}
			}
			
			DocumentNode* pop() {
				DocumentNode* t = top();
				stack.pop_back();
				return t;
			}
			
			void begin_sequence(const char* anchor) {
				auto n = make();
				n->internal_value() = DocumentNode::ArrayType(n->allocator());
				push(n);
				if (anchor != nullptr) anchors[anchor] = top();
			}
			
			void end_sequence() {
				if (state() != Sequence) {
					throw YAMLParserError("Got end of sequence event, but wasn't parsing a sequence.");
				}
				DocumentNode* seq = pop();
				add_value_to_top(seq);
			}
			
			void begin_mapping(const char* anchor) {
				auto n = make();
				n->internal_value() = DocumentNode::MapType(n->allocator());
				push(n);
				if (anchor != nullptr) anchors[anchor] = top();
			}
			
			void end_mapping() {
				if (state() == MappingExpectingValue) {
					throw YAMLParserError("Incomplete mapping.");
				}
				if (state() != MappingExpectingKey) {
					throw YAMLParserError("Got end of mapping event, but wasn't parsing a map.");
				}
				DocumentNode* map = pop();
				add_value_to_top(map);
			}
			
			void alias(const char* anchor) {
				auto it = anchors.find(anchor);
				if (it != anchors.end()) {
					add_value_to_top(it->second);
				} else {
					throw YAMLParserError(String("Undefined anchor: ") + anchor);
				}
			}
			
			void scalar(const char* value_as_string, size_t len) {
				DocumentNode* node = make();
				String input(value_as_string, len);
				if (input == "~") { // Ruby convention for representing nil
					add_value_to_top(node);
					return;
				} else if (input == "\\~") {
					input = "~";
				}
				
				// Check if integer
				bool is_integer = input.size() && input.size() <= 21; // INT64_MIN is 21 characters wide in base 10, including the sign
				if (is_integer) {
					for (auto c: input) {
						if (c < '0' || c > '9') { is_integer = false; break; }
					}
					if (is_integer) {
						Maybe<int64> n = parse<int64>(input);
						*node << n.get_or(0);
						add_value_to_top(node);
						return;
					}
				}
				
				// Check if float
				COPY_STRING_REF_TO_CSTR_BUFFER(buffer, input);
				char* endptr;
				const char* conversion_success_location = buffer.data() + len;
				float64 value = strtod(buffer.data(), &endptr);
				if (endptr == conversion_success_location) {
					*node << value;
				} else {
					// Nope, it was a string!
					*node << std::move(input);
				}
				add_value_to_top(node);
			}
		};
		
		struct YAMLEmitterState {
			const YAMLDocument& document;
			OutputStream& os;
			yaml_emitter_t* emitter;
			YAMLEmitterState(const YAMLDocument& document, OutputStream& os, yaml_emitter_t* emitter) : document(document), os(os), emitter(emitter) {}
			
			int emit(const byte* buffer, size_t sz) {
				os.write(buffer, sz);
				return 1;
			}
			
			void serialize(const DocumentNode* node) {
				node->when<NothingType>([&](NothingType) {
					emit_nil();
				}).when<DocumentNode::ArrayType>([&](const DocumentNode::ArrayType& array) {
					emit_begin_sequence();
					for (auto it: array) {
						serialize(it);
					}
					emit_end_sequence();
				}).when<DocumentNode::MapType>([&](const DocumentNode::MapType& map) {
					emit_begin_mapping();
					for (auto pair: map) {
						emit_string(pair.first);
						serialize(pair.second);
					}
					emit_end_mapping();
				}).when<DocumentNode::FloatType>([&](DocumentNode::FloatType f) {
					StringStream ss;
					ss << f;
					emit_string(ss.str());
				}).when<DocumentNode::IntegerType>([&](DocumentNode::IntegerType n) {
					StringStream ss;
					ss << n;
					emit_string(ss.str());
				}).when<DocumentNode::StringType>([&](StringRef str) {
					if (str == "~") {
						emit_string("\\~");
					} else {
						emit_string(str);
					}
				});
			}
			
			void emit_begin_sequence() {
				yaml_event_t event;
				yaml_sequence_start_event_initialize(&event, nullptr, nullptr, true, YAML_ANY_SEQUENCE_STYLE);
				yaml_emitter_emit(emitter, &event);
			}
			
			void emit_end_sequence() {
				yaml_event_t event;
				yaml_sequence_end_event_initialize(&event);
				yaml_emitter_emit(emitter, &event);
			}
			
			void emit_begin_mapping() {
				yaml_event_t event;
				yaml_mapping_start_event_initialize(&event, nullptr, nullptr, true, YAML_ANY_MAPPING_STYLE);
				yaml_emitter_emit(emitter, &event);
			}
			
			void emit_end_mapping() {
				yaml_event_t event;
				yaml_mapping_end_event_initialize(&event);
				yaml_emitter_emit(emitter, &event);
			}
			
			void emit_string(StringRef str) {
				yaml_event_t event;
				yaml_scalar_event_initialize(&event, nullptr, nullptr, (yaml_char_t*)str.data(), (int)str.size(), true, true, YAML_ANY_SCALAR_STYLE);
				yaml_emitter_emit(emitter, &event);
			}
			
			void emit_nil() {
				yaml_event_t event;
				yaml_scalar_event_initialize(&event, nullptr, nullptr, (yaml_char_t*)"~", 1, true, true, YAML_PLAIN_SCALAR_STYLE);
				yaml_emitter_emit(emitter, &event);
			}
		};
		
		int emit_yaml(void* data, unsigned char* buffer, size_t sz) {
			YAMLEmitterState* state = (YAMLEmitterState*)data;
			return state->emit(buffer, sz);
		}
		
		int yaml_write_handler_t(void *data, unsigned char *buffer, size_t size);
	}
	
	void YAMLDocument::write(OutputStream& os) const {
		yaml_emitter_t emitter;
		yaml_emitter_initialize(&emitter);
		YAMLEmitterState emitter_state(*this, os, &emitter);
		yaml_emitter_set_output(&emitter, emit_yaml, &emitter_state);
		
		yaml_event_t event;
		yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
		yaml_emitter_emit(&emitter, &event);
		
		yaml_document_start_event_initialize(&event, nullptr, nullptr, nullptr, true);
		yaml_emitter_emit(&emitter, &event);
		
		emitter_state.serialize(&root());
		
		yaml_document_end_event_initialize(&event, true);
		yaml_emitter_emit(&emitter, &event);
		
		yaml_stream_end_event_initialize(&event);
		yaml_emitter_emit(&emitter, &event);
		
		yaml_emitter_delete(&emitter);
	}
	
	size_t YAMLDocument::read(InputStream& is, String& out_error) {
		Array<byte> buffer = read_all<Array<byte>>(is);
		const byte* begin = buffer.data();
		const byte* end = begin + buffer.size();
	
		if (end < begin) {
			out_error = "Malformed parameters.";
			return 0;
		}
		if (end == begin) {
			out_error = "No input.";
			return 0;
		}
		
		yaml_parser_t parser;
		if (!yaml_parser_initialize(&parser)) {
			out_error = "libyaml: Failed to initialize parser.";
			return 0;
		}
		
		yaml_parser_set_input_string(&parser, begin, end - begin);
		
		YAMLParserState state(*this);
	
		yaml_event_t event;
		do {
			if (yaml_parser_parse(&parser, &event) == 0) {
				StringStream ss;
				ss << "libyaml: Error " << parser.error << " (" << parser.problem << ") at ";
				ss << "line " << parser.problem_mark.line << ", column " << parser.problem_mark.column << ".";
				out_error = ss.str();
				return 0;
			}
			
			switch (event.type) {
				case YAML_NO_EVENT: break;
				case YAML_STREAM_START_EVENT: break;
				case YAML_STREAM_END_EVENT: break;
				case YAML_DOCUMENT_START_EVENT: break;
				case YAML_DOCUMENT_END_EVENT: break;
				case YAML_SEQUENCE_START_EVENT: state.begin_sequence((const char*)event.data.sequence_start.anchor); break;
				case YAML_SEQUENCE_END_EVENT: state.end_sequence(); break;
				case YAML_MAPPING_START_EVENT: state.begin_mapping((const char*)event.data.mapping_start.anchor); break;
				case YAML_MAPPING_END_EVENT: state.end_mapping(); break;
				case YAML_ALIAS_EVENT: state.alias((const char*)event.data.alias.anchor); break;
				case YAML_SCALAR_EVENT: {
					state.scalar((const char*)event.data.scalar.value, event.data.scalar.length);
					break;
				}
			}
			
			if (event.type != YAML_STREAM_END_EVENT) {
				yaml_event_delete(&event);
			}
		} while (event.type != YAML_STREAM_END_EVENT);
		
		yaml_parser_delete(&parser);
		
		if (state.root() != nullptr) {
			if (!state.root()->is_map()) {
				root().internal_value() = Dictionary<DocumentNode*>({{"yaml_document", state.root()}}, root().allocator());
			} else {
				root().internal_value() = move(state.root()->internal_value());
			}
		} else {
			out_error = "YAML: Unknown error occurred during parsing.";
			return 0;
		}
		
		return end - begin;
	}
	
	bool YAMLDocument::can_parse(const byte* begin, const byte* end) const {
		// There's no way to check but to actually parse it.
		YAMLDocument other;
		String error;
		auto stream = MemoryStream(begin, end);
		if (other.read(stream, error) > 0 && error == "") {
			return true;
		}
		return false;
	}
}
