
# Set output directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBUTILS_SRC IOUtils.cxx IOUtils.h Params.cxx Params.h porter_stemmer.c porter_stemmer.h SimpleLogger.cxx SimpleLogger.h)

ADD_LIBRARY(utils STATIC ${LIBUTILS_SRC})

#set properties on the libraries
SET_TARGET_PROPERTIES(utils PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

INSTALL(TARGETS utils
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

