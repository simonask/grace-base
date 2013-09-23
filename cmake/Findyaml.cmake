# - Try to find libyaml

find_package(PkgConfig)
pkg_check_modules(PC_YAML QUIET yaml-0.1)
set(YAML_DEFINITIONS ${PC_YAML_CFLAGS_OTHER})

find_path(YAML_INCLUDE_DIR yaml.h HINTS ${PC_YAML_INCLUDEDIR} ${PC_YAML_INCLUDE_DIRS})

find_library(YAML_LIBRARY NAMES yaml HINTS ${PC_YAML_LIBDIR} ${PC_YAML_LIBRARY_DIRS})

set(YAML_LIBRARIES ${YAML_LIBRARY})
set(YAML_INCLUDE_DIRS ${YAML_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml DEFAULT_MSG YAML_LIBRARY YAML_INCLUDE_DIR)
mark_as_advanced(YAML_INCLUDE_DIR YAML_LIBRARY)