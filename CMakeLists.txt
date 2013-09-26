cmake_minimum_required(VERSION 2.8)
set(SRCPATH .)
list(APPEND CMAKE_MODULE_PATH ./cmake)

set(CMAKE_CXX_FLAGS "-O0 -g -std=c++11 -stdlib=libc++ -DDEBUG -Wno-unused-comparison")

include_directories(${SRCPATH})

set(SOURCES
	base/any.cpp
	base/arch.cpp
	base/array.cpp
	base/array_list.cpp
	base/array_ref.cpp
	base/bag.cpp
	base/benchmark.cpp
	base/color.cpp
	base/config.cpp
	base/error.cpp
	base/exceptions.cpp
	base/fiber.cpp
	base/function.cpp
	base/log.cpp
	base/process.cpp
	base/random.cpp
	base/regex.cpp
	base/string.cpp
	base/time.cpp
	base/type_info.cpp
	geometry/rect.cpp
	geometry/units.cpp
	geometry/vector.cpp
	io/archive.cpp
	io/builtin_archive.cpp
	io/fd.cpp
	io/file_stream.cpp
	io/formatted_stream.cpp
	io/memory_stream.cpp
	io/network_stream.cpp
	io/pipe_stream.cpp
	io/resource.cpp
	io/resource_loader.cpp
	io/resource_manager.cpp
	io/stdio_stream.cpp
	io/string_stream.cpp
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
	tests/test.cpp
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

find_package(yaml REQUIRED)
find_package(event REQUIRED)

include_directories(${YAML_INCLUDE_DIRS})
include_directories(${EVENT_INCLUDE_DIRS})

list(APPEND SOURCES platform/libevent_util.cpp platform/event_loop_libevent.cpp)

add_library(grace-base SHARED ${FINAL_SOURCES})

target_link_libraries(grace-base ${YAML_LIBRARIES})
target_link_libraries(grace-base ${EVENT_LIBRARIES})


set(TESTS
	anim_utils_test
	any_test
	array_list_test
	array_ref_test
	array_test
	aspect_cast_test
	binary_archive_test
	composite_test
	either_test
	error_test
	fiber_test
	formatting_test
	function_test
	geometry_test
	link_list_test
	map_test
	math_test
	matrix_test
	maxarray_test
	maybe_test
	memory_stream_test
	network_stream_test
	priority_queue_test
	process_test
	regex_test
	signal_test
	simd_test
	string_test
	time_test
	type_info_test
	vector_test
	work_queue_test
)

add_custom_target(test)

foreach(t ${TESTS})
	add_executable(.tests/${t} EXCLUDE_FROM_ALL tests/${t}.cpp)
	target_link_libraries(.tests/${t} grace-base)
	add_custom_target(run_${t} COMMAND .tests/${t} $ENV{TEST_FLAGS})
	add_dependencies(run_${t} .tests/${t})
	add_dependencies(test run_${t})
endforeach()
