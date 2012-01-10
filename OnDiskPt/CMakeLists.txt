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

SET(LIBODPT_SRC
    OnDiskWrapper.cpp OnDiskWrapper.h
    Phrase.cpp Phrase.h
    PhraseNode.cpp PhraseNode.h
    SourcePhrase.cpp SourcePhrase.h
    TargetPhraseCollection.cpp TargetPhraseCollection.h
    TargetPhrase.cpp TargetPhrase.h
    Vocab.cpp Vocab.h
    Word.cpp Word.h
)




ADD_LIBRARY(ondiskpt STATIC ${LIBODPT_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/util )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lm )

#set properties on the libraries
SET_TARGET_PROPERTIES(ken PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

#BUILD BINARY

SET( ONDISKPT_SRC Main.cpp Main.h )

ADD_EXECUTABLE(ondisk_pt ${ONDISKPT_SRC} )

TARGET_LINK_LIBRARIES (
    ondisk_pt
    ondiskpt
    moseslib
    ken
    util
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )


INSTALL(TARGETS ondiskpt ondisk_pt 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

