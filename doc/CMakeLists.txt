# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2023 Meta Platforms, Inc. and affiliates.

configure_file(Doxyfile.in Doxyfile)
configure_file(conf.py.in conf.py)

set(doc_srcs
	index.rst reference.rst todo.rst
)

add_custom_target(doc
	COMMAND Doxygen::doxygen Doxyfile
	COMMAND ${SPHINX_BIN} -c . -Dbreathe_projects.bpfilter=xml ${CMAKE_CURRENT_SOURCE_DIR} html
	DEPENDS Doxyfile.in conf.py ${doc_srcs} ${CMAKE_SOURCE_DIR}/src/main.c ${bpfilter_srcs}
	BYPRODUCTS xml html
	COMMENT "Generate HTML documentation"
)

add_custom_target(coverage
	COMMAND ${LCOV_BIN} --capture --directory ${CMAKE_BINARY_DIR} --output-file lcov.out
	COMMAND ${LCOV_BIN} --output-file lcov.out --remove lcov.out "${CMAKE_SOURCE_DIR}/tests/unit/\\*"
	COMMAND ${GENHTML_BIN} -o coverage lcov.out
	BYPRODUCTS coverage
	COMMENT "Generate coverage report"
)
