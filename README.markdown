Grace Engine Base Library
===========================

This library contains a number of general-purpose classes for use in various high-performance/low-latency
projects. Let's get it out of the way: It competes with the STL in a number of areas, but interoperation
is a high priority.

Rather than duplicating STL functionality 1-to-1, the Grace Base library has a different take on a number
of concepts, and also adds features that the STL doesn't yet define portably.

The library contains the following toplevel components:

* **Base library:** Containers, algorithms, utilities, support for various idioms.
* **Geometry library:** Vector and Matrix support, with explicit but portable usage of SIMD instructions where applicable.
* **I/O library:** Streams, files, sockets, resource management, formatting, reactive programming.
* **Memory library:** Various allocator idioms.
* **Object library:** Object support systems, implementing reflection, introspection, limited runtime metaprogramming, and composite types.
* **Serialization library:** Marshalling of basic types, containers, and arbitrary objects using the reflection mechanisms of the Object library. Grace includes serializers and deserializers for YAML and JSON.
* **Test library:** A compact unit testing framework.

Design Philosophy
-----------------

The main overlap with STL is in the implementation of the containers in the base library. There are
a few reasons for this overlap:

1. STL containers, while great in most implementations, can be difficult to introspect and debug. They
can also perform very differently across different platforms, to the point where many large time-critical
projects (such as pseudo-realtime video games and such) often go for a non-standard implementation of the
STL anyway.

2. The STL is designed for the general case. While it can be specialized to perform well under very
narrow constraints, doing so is often clunky and difficult to work with. This is especially true
with the allocator patterns used in the STL, which is decided at compile time: It performs very well,
but encoded memory allocation pattern in the type of an object raises a range of complications that
users of the library don't necessarily want to deal with to leverage the performance benefits of a specialized
allocator. Therefore, Grace opts for a dynamic-dispatch approach to memory allocation, through the `IAllocator` interface.

3. While Grace distances itself from STL in a number of decisions, it remains interoperable. For instance,
all algorithms from the STL can be used with Grace containers at zero extra cost. Memory allocators from the Memory library
can be used with STL containers with the `STLAllocator` class.

To keep binary size down, Grace does not come with reflection and serialization bindings for STL containers, but
these are trivial to implement if necessary.
