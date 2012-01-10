# Set output directory
FIND_PACKAGE(ZLIB)

IF(ZLIB_FOUND)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${ZLIB_LIBRARY_DIRS})
    LINK_DIRECTORIES( ${BZIP2_LIBRARY_DIRS})
    LINK_DIRECTORIES( ${ZLIB_LIBRARY_DIRS})
ELSE()
    MESSAGE(FATAL_ERROR "ZLIB not found" )
ENDIF (ZLIB_FOUND)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBKEN_SRC
    bhiksha.cc bhiksha.hh
    binary_format.cc binary_format.hh
    blank.hh 
    config.cc config.hh
    enumerate_vocab.hh facade.hh
    left.hh left_test.cc
    lm_exception.cc lm_exception.hh
    max_order.hh model.cc
    model.hh model_test.cc
    model_type.hh 
    ngram_query.hh quantize.cc
    quantize.hh read_arpa.cc
    read_arpa.hh return.hh
    search_hashed.cc search_hashed.hh
    search_trie.cc search_trie.hh
    trie.cc trie.hh
    trie_sort.cc trie_sort.hh
    virtual_interface.cc virtual_interface.hh
    vocab.cc vocab.hh
    weights.hh word_index.hh
)



ADD_LIBRARY(ken STATIC ${LIBKEN_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/util )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lm )

#set properties on the libraries
SET_TARGET_PROPERTIES(ken PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

#BUILD BINARY

SET( BUILDBINARY_SRC build_binary.cc )

ADD_EXECUTABLE(build_binary ${BUILDBINARY_SRC} )

TARGET_LINK_LIBRARIES (
    build_binary
    ken
    util
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )

#NGRAM Query

SET( NGQUERY_SRC ngram_query.cc )

ADD_EXECUTABLE(ngram_query ${NGQUERY_SRC} )

TARGET_LINK_LIBRARIES (
    ngram_query
    ken
    util
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )



INSTALL(TARGETS ken ngram_query build_binary 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

