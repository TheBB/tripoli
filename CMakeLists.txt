cmake_minimum_required(VERSION 3.0)

enable_language(C)

find_path(EMACS_INCLUDE_DIR NAMES emacs-module.h PATH_SUFFIXES src)
if(NOT EMACS_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find emacs-module.h.")
endif()

find_package(PythonLibs 3.4 REQUIRED)

include(CheckCCompilerFlag)
function(enable_c_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_C_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_c_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

enable_c_compiler_flag_if_supported("-Wall")
enable_c_compiler_flag_if_supported("-Wextra")
enable_c_compiler_flag_if_supported("-pedantic")

add_library(tripoli SHARED lib/main.c lib/emacs-interface.c lib/module.c lib/object.c lib/error.c)
install(TARGETS tripoli LIBRARY DESTINATION /usr/share/emacs/site-lisp)
set_property(TARGET tripoli PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(tripoli PRIVATE
  ${EMACS_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(tripoli ${PYTHON_LIBRARIES})

add_custom_target(run COMMAND emacs -q -L . -l libtripoli VERBATIM)
add_dependencies(run tripoli)

add_custom_target(run-bare COMMAND emacs -q -L .
  --eval "(setq tripoli-inhibit-init t)"
  -l libtripoli VERBATIM)
add_dependencies(run tripoli)

add_custom_target(repl COMMAND
  emacs --batch -q -L .
  --eval "(setq tripoli-inhibit-init t)"
  -l libtripoli
  --eval "(tripoli-repl)"
  --eval "(kill-emacs)" VERBATIM)
add_dependencies(repl tripoli)

add_custom_target(docs COMMAND python3 docs.py)
add_dependencies(docs tripoli)
configure_file(docs.py docs.py)

add_custom_target(gh-pages COMMAND python3 gh-pages.py)
add_dependencies(gh-pages tripoli)
configure_file(gh-pages.py gh-pages.py)

# Test targets

add_custom_target(check COMMAND
  emacs --batch -q -L .
  --eval "(setq tripoli-inhibit-init t)"
  -l libtripoli
  --eval "(kill-emacs (tripoli-test))" VERBATIM)
add_dependencies(check tripoli)
