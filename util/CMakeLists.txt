# Set output directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBUTIL_SRC 
bit_packing.cc bit_packing.hh
bit_packing_test.cc check.hh
ersatz_progress.cc ersatz_progress.hh
exception.cc exception.hh
file.cc file.hh
file_piece.cc file_piece.hh
file_piece_test.cc getopt.c
getopt.hh have.hh
joint_sort.hh joint_sort_test.cc
key_value_packing.hh key_value_packing_test.cc
mmap.cc mmap.hh
murmur_hash.cc murmur_hash.hh
probing_hash_table.hh probing_hash_table_test.cc
proxy_iterator.hh scoped.hh
sized_iterator.hh sorted_uniform.hh
sorted_uniform_test.cc string_piece.hh
tokenize_piece.hh tokenize_piece_test.cc
    )


ADD_LIBRARY(util STATIC ${LIBUTIL_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/moses/src )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/contrib/synlm/hhmm/wsjparse/include/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/contrib/synlm/hhmm/rvtl/include/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/util )

#set properties on the libraries
SET_TARGET_PROPERTIES(util PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

INSTALL(TARGETS util
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

