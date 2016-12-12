cmake_minimum_required(VERSION 3.0)

enable_language(C)

find_path(EMACS_INCLUDE_DIR NAMES emacs-module.h PATH_SUFFIXES src)
if(NOT EMACS_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find emacs-module.h.")
endif()

find_package(PythonLibs 3.4 REQUIRED)

add_library(tripoli SHARED lib/main.c lib/interface.c lib/module.c lib/EmacsObject.c lib/error.c)
install(TARGETS tripoli LIBRARY DESTINATION lib)
set_property(TARGET tripoli PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(tripoli PRIVATE
  ${EMACS_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(tripoli ${PYTHON_LIBRARIES})

add_custom_target(repl COMMAND ./repl)
add_dependencies(repl tripoli)
configure_file(repl repl)

add_custom_target(check COMMAND ./test)
add_dependencies(check tripoli)
configure_file(test test)

add_custom_target(doc COMMAND ./doc)
add_dependencies(doc tripoli)
configure_file(doc doc)
