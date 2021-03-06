CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(sflphone-client-kde)

SET(LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" )
SET(CMAKE_MODULE_PATH       "${LOCAL_CMAKE_MODULE_PATH}" )

SET(QT_USE_QT*)

if(POLICY CMP0017)
   cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)


# --- custom targets: ---
IF(NOT (${ENABLE_QT5} MATCHES true))
   FIND_PACKAGE ( KdepimLibs REQUIRED                                                            )
   FIND_PACKAGE ( KDE4       REQUIRED                                                            )
   FIND_PACKAGE ( Qt4        REQUIRED QtCore QtGui QtXml QtDBus QtTest QtSVG QtOpenGl QT_USE_QT* )
ENDIF(NOT (${ENABLE_QT5} MATCHES true))

# GCC 4.5 is too old for C++11
if (CMAKE_COMPILER_IS_GNUCC)
   execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
      message(STATUS "Found GCC version >= 4.8: " ${GCC_VERSION})
   else()
      message(FATAL_ERROR "Your version of GCC is too old, please install GCC 4.6 or later")
   endif()
endif()

IF(${ENABLE_QT5} MATCHES true)
   add_subdirectory( src/lib )
ELSE()
   set(AKONADI_MIN_VERSION 1.0)
   find_package(Akonadi QUIET NO_MODULE ${AKONADI_MIN_VERSION})

   INCLUDE ( KDE4Defaults )

   MESSAGE("cmake install prefix is : ${CMAKE_INSTALL_PREFIX}")

   INCLUDE_DIRECTORIES (${KDE4_INCLUDES} ${QT_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

   INCLUDE (  ${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake  )
   INCLUDE(   ${QT_USE_FILE}   )
   add_subdirectory( data    )
   add_subdirectory( src     )
   add_subdirectory( doc     )
   add_subdirectory( man     )
ENDIF(${ENABLE_QT5} MATCHES true)

IF(${ENABLE_TEST} MATCHES true)
   add_subdirectory( src/test)
ENDIF()

IF(EXISTS "po" AND IS_DIRECTORY "po")
   FIND_PACKAGE ( Gettext REQUIRED )
   add_subdirectory( po      )
ENDIF()

set(PROJECT_VERSION "1.4.1")
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})

add_custom_target(dist
      COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
      | gzip > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
