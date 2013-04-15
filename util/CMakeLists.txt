# Set output directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBUTIL_SRC 
    mmap.cc
    stream/chain.cc
    stream/multi_progress.cc
    stream/line_input.cc
    stream/io.cc
    double-conversion/fast-dtoa.cc
    double-conversion/diy-fp.cc
    double-conversion/double-conversion.cc
    double-conversion/strtod.cc
    double-conversion/fixed-dtoa.cc
    double-conversion/cached-powers.cc
    double-conversion/bignum.cc
    double-conversion/bignum-dtoa.cc
    pool.cc
    file_piece.cc
    file.cc
    read_compressed.cc
    exception.cc
    scoped.cc
    ersatz_progress.cc
    string_piece.cc
    bit_packing.cc
    murmur_hash.cc
    usage.cc
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

