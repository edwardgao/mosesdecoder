
# Set output directory

FIND_PACKAGE(Threads)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET( LIBSRL_SRC
 aligninfo.cc
 aligninfo.h
 ruleextract.cc
 ruleextract.h
 srlinfo.cc
 srlinfo.h
 srlinfoparser.cc
 srlinfoparser.h
 srlruleset.cc
 srlruleset.h
 srlmodel.cpp
 srlmodel.h
)

ADD_LIBRARY(srl_static STATIC ${LIBSRL_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/srl/utils )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/utils )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/phrase-extract )
LINK_DIRECTORIES ( ${LIBRARY_OUTPUT_PATH} )

SET( SRLEXTRACT_SRC srlextract.cpp )

ADD_EXECUTABLE( srlextract ${SRLEXTRACT_SRC} )

TARGET_LINK_LIBRARIES (
    srlextract
	srl_static
	utils
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

SET( RESCORE_NBEST_SRC rescore-nbest-srl.cpp )

ADD_EXECUTABLE( rescore-nbest-srl ${RESCORE_NBEST_SRC} )

TARGET_LINK_LIBRARIES (
    rescore-nbest-srl
	srl_static
	phex
	utils	
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

SET( MONOEXTRACT_SRC monoextract.cpp )

ADD_EXECUTABLE( monoextract ${MONOEXTRACT_SRC} )

TARGET_LINK_LIBRARIES (
    monoextract
	srl_static
	utils
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )


INSTALL(TARGETS srlextract monoextract srl_static rescore-nbest-srl
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )



