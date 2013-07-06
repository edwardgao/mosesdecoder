# Set output directory
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBPHEX_SRC
    AlignmentPhrase.cpp
	AlignmentPhrase.h
    ExtractedRule.cpp

    HoleCollection.cpp
	HoleCollection.h
    InputFileStream.cpp
	InputFileStream.h
    OutputFileStream.cpp
    PhraseAlignment.cpp
	SafeGetline.h
    ScoreFeature.cpp
    ScoreFeatureTest.cpp
    SentenceAlignment.cpp
    SentenceAlignmentWithSyntax.cpp
    SyntaxTree.cpp
	SyntaxTree.h
    tables-core.cpp
	tables-core.h
	RuleExtractionOptions.h
	PhraseExtractionOptions.h
    domain.cpp
	domain.h
    XmlTree.cpp                                      
	ExtractedRule.h    
	OutputFileStream.h
	score.h
	XmlException.h
	extract-lex.h
	PhraseAlignment.h
	ScoreFeature.h
	XmlTree.h
	gzfilebuf.h
	SentenceAlignment.h
	hierarchical.h
	relax-parse.h
	SentenceAlignmentWithSyntax.h
	Hole.h
	RuleExist.h
	srlbridge.h
	srlbridge.cpp
)


ADD_LIBRARY(phex STATIC ${LIBPHEX_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/util )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/srl/utils )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lm )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/srl/srlextract )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/search )

#set properties on the libraries
SET_TARGET_PROPERTIES(phex PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

#BUILD BINARY
#consolidate-direct-main.cpp  consolidate-reverse-main.cpp  domain.h              extract-main.cpp        relax-parse-main.cpp  statistics-main.cpp
#consolidate-main.cpp         domain.cpp                    extract-lex-main.cpp  extract-rules-main.cpp  score-main.cpp        test.domain

ADD_EXECUTABLE(  score score-main.cpp )

TARGET_LINK_LIBRARIES (
    score
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE(  extract-rules extract-rules-main.cpp )

TARGET_LINK_LIBRARIES (
    extract-rules
    phex
    moseslib
    srl_static
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE( srl-model-estimate srl-model-estimate.cpp )

TARGET_LINK_LIBRARIES (
    srl-model-estimate
    phex
    moseslib
    srl_static
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE(  extract-lex extract-lex-main.cpp )

TARGET_LINK_LIBRARIES (
    extract-lex
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )
ADD_EXECUTABLE( consolidate consolidate-main.cpp )

TARGET_LINK_LIBRARIES (
    consolidate
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )
ADD_EXECUTABLE( statistics statistics-main.cpp )

TARGET_LINK_LIBRARIES (
    statistics
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE( relax-parse relax-parse-main.cpp )

TARGET_LINK_LIBRARIES (
    relax-parse
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE( extract extract-main.cpp )

TARGET_LINK_LIBRARIES (
    extract
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE( consolidate-reverse consolidate-reverse-main.cpp )

TARGET_LINK_LIBRARIES (
    consolidate-reverse
    phex
    moseslib
    ken
    util
    utils
    search
	lmert
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
   )

ADD_EXECUTABLE( consolidate-direct consolidate-direct-main.cpp )

TARGET_LINK_LIBRARIES (
    consolidate-direct
    phex
    moseslib
    ken
    util
    utils
    search
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )


INSTALL(TARGETS phex consolidate-direct consolidate-reverse extract
    extract-rules extract-lex consolidate statistics score srl-model-estimate
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

