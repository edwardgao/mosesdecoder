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

IF(WIN32)
	SET(VCCOMPLIB vccomp)
ENDIF(WIN32)

SET(LIBMERT_SRC
    BleuScorer.cpp BleuScorer.h
    CderScorer.cpp CderScorer.h
    Data.cpp Data.h
    FeatureArray.cpp FeatureArray.h
    FeatureData.cpp FeatureData.h
    FeatureDataIterator.cpp FeatureDataIterator.h
    FeatureStats.cpp FeatureStats.h
    FileStream.cpp FileStream.h
    gzfilebuf.h 
    MergeScorer.cpp  MergeScorer.h
    Optimizer.cpp Optimizer.h
    PerScorer.cpp PerScorer.h
    Point.cpp Point.h
    ScopedVector.h
    ScoreArray.cpp ScoreArray.h
    ScoreData.cpp ScoreData.h
    ScoreDataIterator.cpp ScoreDataIterator.h
    Scorer.cpp ScorerFactory.cpp
    ScorerFactory.h Scorer.h
    ScoreStats.cpp ScoreStats.h
    TerScorer.cpp TerScorer.h
    Timer.cpp
    Timer.h Types.h
    Util.cpp Util.h
    TERsrc/alignmentStruct.cpp TERsrc/alignmentStruct.h
    TERsrc/bestShiftStruct.h TERsrc/hashMap.cpp
    TERsrc/hashMap.h TERsrc/hashMapInfos.cpp
    TERsrc/hashMapInfos.h TERsrc/hashMapStringInfos.cpp
    TERsrc/hashMapStringInfos.h TERsrc/infosHasher.cpp
    TERsrc/infosHasher.h TERsrc/stringHasher.cpp
    TERsrc/stringHasher.h TERsrc/stringInfosHasher.cpp
    TERsrc/stringInfosHasher.h TERsrc/terAlignment.cpp
    TERsrc/terAlignment.h TERsrc/tercalc.cpp
    TERsrc/tercalc.h TERsrc/terShift.cpp
    TERsrc/terShift.h TERsrc/tools.cpp
    TERsrc/tools.h
)


ADD_LIBRARY(lmert STATIC ${LIBMERT_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/mert/TERsrc )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/utils )

#set properties on the libraries
SET_TARGET_PROPERTIES(lmert PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

#MERT

SET( MERT_SRC mert.cpp )

IF(ENABLE_MULTI_THREAD)
    SET(MERT_THREAD_SRC ../moses/src/ThreadPool.cpp ../moses/src/ThreadPool.h)
ENDIF()

ADD_EXECUTABLE(mert ${MERT_SRC} ${MERT_THREAD_SRC})

TARGET_LINK_LIBRARIES (
    mert
    lmert
    util
	${VCCOMPLIB}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )

#EVALUATOR

SET( EVALUATOR_SRC evaluator.cpp )

ADD_EXECUTABLE(evaluator ${EVALUATOR_SRC} )

TARGET_LINK_LIBRARIES (
    evaluator
    lmert
    util
	${VCCOMPLIB}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )


#EXTRACTOR

SET( EXTRACTOR_SRC extractor.cpp )

ADD_EXECUTABLE(extractor ${EXTRACTOR_SRC} )

TARGET_LINK_LIBRARIES (
    extractor
    lmert
    util
	${VCCOMPLIB}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )


#PRO

SET( PRO_SRC pro.cpp )

ADD_EXECUTABLE(pro ${PRO_SRC} )

TARGET_LINK_LIBRARIES (
    pro
    lmert
    util
	${VCCOMPLIB}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )


#PRO

SET( TESTSCORER_SRC test_scorer.cpp )

ADD_EXECUTABLE(test_scorer ${TESTSCORER_SRC} )

TARGET_LINK_LIBRARIES (
    test_scorer
    lmert
    util
	${VCCOMPLIB}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )



INSTALL(TARGETS lmert mert pro extractor evaluator test_scorer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

