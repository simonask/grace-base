# - Try to find libevent

find_package(PkgConfig)
pkg_check_modules(PC_EVENT QUIET event)
set(EVENT_DEFINITIONS ${PC_EVENT_CFLAGS_OTHER})

find_path(EVENT_INCLUDE_DIR event2/event.h HINTS ${PC_EVENT_INCLUDEDIR} ${PC_EVENT_INCLUDE_DIRS})

find_library(EVENT_LIBRARY NAMES event HINTS ${PC_EVENT_LIBDIR} ${PC_EVENT_LIBRARY_DIRS})

set(EVENT_LIBRARIES ${EVENT_LIBRARY})
set(EVENT_INCLUDE_DIRS ${EVENT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(event DEFAULT_MSG EVENT_LIBRARY EVENT_INCLUDE_DIR)
mark_as_advanced(EVENT_INCLUDE_DIR EVENT_LIBRARY)