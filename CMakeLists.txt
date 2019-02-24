cmake_minimum_required (VERSION 2.8)

project ( AISL C)

set (PROJECT_TITLE "aisl")
set (LIBRARY_NAME  ${PROJECT_TITLE})
# set (DEMO_NAME   "demo")

# Defaults --------------------------------------------------------------------

include (cmake.compiler)
include (cmake.version)
include (cmake.system)
include (cmake.paths)

# Definitions -----------------------------------------------------------------

add_definitions(
  -DPROJECT_TITLE="${PROJECT_TITLE}"
)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fvisibility=hidden")

if(DEFINED CMAKE_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror ")
  add_definitions( -DDEBUG=${CMAKE_DEBUG} )
endif()


# Options ---------------------------------------------------------------------

#if( DEFINED WITH_EVERYTHING )
#  set(WITH_TEMPLIGHT 1)
#  set(WITH_OPTIONS 1)
#  set(WITH_CONFIG 1)
#endif()

# Sources ---------------------------------------------------------------------

include_directories( "." ${INCLUDE_DIR} )
  
add_definitions( -DLIST_WITH_APPEND
                 -DCSTUFF_LIST_WITH_REMOVE_INDEX
                 -DCSTUFF_LIST_WITH_REMOVE
                 -DCSTUFF_LIST_WITH_APPEND
                 -DCSTUFF_LIST_WITH_INSERT
                 -DCSTUFF_LIST_WITH_COPY
                 -DCSTUFF_STR_UTILS_WITH_COPY
                 -DCSTUFF_STR_UTILS_WITH_NCAT
                 -DCSTUFF_STR_UTILS_WITH_CMPI
                 -DCSTUFF_STR_UTILS_WITH_PRINTF
                 -DCSTUFF_STR_UTILS_WITH_NCOPY
                 -DSTR_UTILS_WITH_COPY
                 -DSTR_UTILS_WITH_NCOPY
                 -DSTR_UTILS_WITH_CAT
                 -DSTR_UTILS_WITH_PRINTF
                 -DSTR_UTILS_WITH_CMPI
)

set ( LIBRARY_SOURCES library/aisl.c
                      library/buffer.c 
                      library/server.c 
                      library/client.c 
                      library/stream.c
                      library/parser.c 
                      library/http.c 
                      library/handle.c 
                      library/status.c 
                      library/event.c 
                      cStuff/list.c
                      cStuff/str-utils.c)

set ( DEMO_SOURCES demo/main.c demo/events.c demo/urls.c )

set ( META_FILES README.md LICENSE.md AUTHORS.md)

#if( DEFINED WITH_TEMPLIGHT )
#  set(SOURCE_FILES ${SOURCE_FILES} ${SOURCES_DIR}/templight.c)
#  add_definitions( -DWITH_TEMPLIGHT )
#endif()

#if( DEFINED WITH_OPTIONS )
#  set(SOURCE_FILES ${SOURCE_FILES} ${SOURCES_DIR}/options.c)
#  add_definitions( -DWITH_OPTIONS )
#endif()

#if( DEFINED WITH_CONFIG )
#  set(SOURCE_FILES ${SOURCE_FILES} ${SOURCES_DIR}/config.c)
#  add_definitions( -DWITH_CONFIG )
#endif()

find_package(OpenSSL)

include_directories(
  ${OPENSSL_INCLUDE_DIRS}
)

link_directories(
  ${OPENSSL_LIBRARY_DIRS}
)

# Library ----------------------------------------------------------------------

add_library(${LIBRARY_NAME} SHARED ${LIBRARY_SOURCES})

# Demos ------------------------------------------------------------------------

#add_executable(${DEMO_NAME} ${DEMO_SOURCES})

#target_link_libraries(${DEMO_NAME} ${LIBRARY_NAME})
target_link_libraries(${LIBRARY_NAME} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

# Installation ----------------------------------------------------------------

install(
  TARGETS ${LIBRARY_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
#  ${LIB_INSTALL_DIR} 

#install(
#  FILES ${META_FILES}
#  DESTINATION ${SHARE_INSTALL_PREFIX}/doc/packages/${LIBRARY_NAME}/
#)

install(
  DIRECTORY ${INCLUDE_DIR}/${LIBRARY_NAME} 
  DESTINATION ${INCLUDE_INSTALL_DIR}
)


