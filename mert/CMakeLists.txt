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
    Util.cpp
    GzFileBuf.cpp
    FileStream.cpp
    Timer.cpp
    ScoreStats.cpp
    ScoreArray.cpp
    ScoreData.cpp
    ScoreDataIterator.cpp
    FeatureStats.cpp
    FeatureArray.cpp
    FeatureData.cpp
    FeatureDataIterator.cpp
    MiraFeatureVector.cpp
    MiraWeightVector.cpp
    HypPackEnumerator.cpp
    Data.cpp
    BleuScorer.cpp
    SemposScorer.cpp
    SemposOverlapping.cpp
    InterpolatedScorer.cpp
    Point.cpp
    PerScorer.cpp
    Scorer.cpp
    ScorerFactory.cpp
    Optimizer.cpp
    OptimizerFactory.cpp
    TER/alignmentStruct.cpp
    TER/hashMap.cpp
    TER/hashMapStringInfos.cpp
    TER/stringHasher.cpp
    TER/terAlignment.cpp
    TER/terShift.cpp
    TER/hashMapInfos.cpp
    TER/infosHasher.cpp
    TER/stringInfosHasher.cpp
    TER/tercalc.cpp
    TER/tools.cpp
    TerScorer.cpp
    CderScorer.cpp
    Vocabulary.cpp
    PreProcessFilter.cpp
    SentenceLevelScorer.cpp
    Permutation.cpp
    PermutationScorer.cpp
    StatisticsBasedScorer.cpp
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
    SET(MERT_THREAD_SRC ../moses/ThreadPool.cpp ../moses/ThreadPool.h)
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

#SET( TESTSCORER_SRC test_scorer.cpp )

#ADD_EXECUTABLE(test_scorer ${TESTSCORER_SRC} )

#TARGET_LINK_LIBRARIES (
#    test_scorer
#    lmert
#    util#
#	${VCCOMPLIB}
#    ${ZLIB_LIBRARIES}
#    ${Boost_LIBRARIES}
#    )



INSTALL(TARGETS lmert mert pro extractor evaluator 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

