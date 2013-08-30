cmake_minimum_required(VERSION 2.8)
set(SRCPATH .)
#include(${LEVEL}/config.cmake)

set(CMAKE_CXX_FLAGS "-O0 -g -std=c++11 -stdlib=libc++ -DDEBUG")

include_directories(${SRCPATH})

set(SOURCES
	base/any.cpp
	base/arch.cpp
	base/array.cpp
	base/array_ref.cpp
	base/bag.cpp
	base/benchmark.cpp
	base/color.cpp
	base/config.cpp
	base/exceptions.cpp
	base/fiber.cpp
	base/log.cpp
	base/random.cpp
	base/regex.cpp
	base/string.cpp
	base/time.cpp
	event/event_loop_libevent.cpp
	event/libevent_util.cpp
	geometry/rect.cpp
	geometry/units.cpp
	geometry/vector.cpp
	io/archive.cpp
	io/builtin_archive.cpp
	io/file_stream.cpp
	io/formatted_stream.cpp
	io/memory_stream.cpp
	io/process_stream.cpp
	io/resource.cpp
	io/resource_loader.cpp
	io/resource_manager.cpp
	io/stdio_stream.cpp
	io/util.cpp
	loaders/object_template_loader.cpp
	memory/allocator.cpp
	memory/memory_tracker.cpp
	memory/static_allocator.cpp
	object/composite_type.cpp
	object/editor_universe.cpp
	object/object.cpp
	object/object_template.cpp
	object/object_type.cpp
	object/property_animator.cpp
	object/signal.cpp
	object/slot.cpp
	object/universe.cpp
	object/universe_base.cpp
	serialization/binary.cpp
	serialization/deserialize_object.cpp
	serialization/document.cpp
	serialization/document_node.cpp
	serialization/json.cpp
	serialization/yaml.cpp
	type/any_type.cpp
	type/array_type.cpp
	type/attribute.cpp
	type/boolean_type.cpp
	type/color_type.cpp
	type/dictionary_type.cpp
	type/map_type.cpp
	type/matrix_type.cpp
	type/maybe_type.cpp
	type/pointer_type.cpp
	type/reference_type.cpp
	type/structured_type.cpp
	type/type.cpp
	type/type_registry.cpp
	grace_base.cpp
)

set(FINAL_SOURCES)
foreach(f ${SOURCES})
	list(APPEND FINAL_SOURCES ${SRCPATH}/${f})
endforeach(f)

find_library(YAML_LIB yaml)
find_library(EVENT_LIB event)

add_library(grace-base SHARED ${FINAL_SOURCES})

target_link_libraries(grace-base yaml)
target_link_libraries(grace-base event)
