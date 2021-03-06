# Set output directory
FIND_PACKAGE(ZLIB)

IF(ZLIB_FOUND)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
    LINK_DIRECTORIES( ${ZLIB_LIBRARY_DIRS})
ELSE()
    MESSAGE(FATAL_ERROR "ZLIB not found" )
ENDIF (ZLIB_FOUND)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/util )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/moses )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/search )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lm )


#BUILD BINARY

SET( MOSESCHART_SRC Main.cpp mbr.cpp IOWrapper.cpp TranslationAnalysis.cpp) 

ADD_EXECUTABLE(moseschart ${MOSESCHART_SRC} )

TARGET_LINK_LIBRARIES (
    moseschart
    moseslib
    ondiskpt
    search
    ken
    util
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )

#set properties on the libraries
SET_TARGET_PROPERTIES(moseschart PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)


INSTALL(TARGETS moseschart 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

