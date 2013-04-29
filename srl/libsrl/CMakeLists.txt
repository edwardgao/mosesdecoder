
# Set output directory

FIND_PACKAGE(Threads)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#ADD_DEFINITIONS("-DNDEBUG")

#IF (WIN32)

#ELSE()
#ADD_DEFINITIONS("-Wno-deprecated")
#ADD_DEFINITIONS("-Wno-write-strings")
#ENDIF()

SET( LIBSRL_SRC
	srl.cpp srl.h
	predtable.cpp predtable.h
	argtable.cpp argtable.h
	srlfeat.cpp srlfeat.h
	srlio.h
	readers.h readers.cpp
	parsers.h parsers.cpp
	writers.h writers.cpp
	formatters.h formatters.cpp
	defs.h
)

ADD_LIBRARY(srl STATIC ${LIBSRL_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )


IF (WIN32)
	INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/w32 )
	LINK_DIRECTORIES ( ${PROJECT_SOURCE_DIR}/w32 )
	IF( USE_64_BIT )
		SET(CMAKE_THREAD_LIBS_INIT pthread64)
	ELSE( USE_64_BIT )
		SET(CMAKE_THREAD_LIBS_INIT pthread)
	ENDIF(  )	
ENDIF()	

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/srl/ )

LINK_DIRECTORIES ( ${LIBRARY_OUTPUT_PATH} )


INSTALL(TARGETS srl 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)



