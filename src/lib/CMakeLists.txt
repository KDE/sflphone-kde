CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if(POLICY CMP0022)
   cmake_policy(SET CMP0022 NEW)
endif(POLICY CMP0022)

INCLUDE(GNUInstallDirs)

ADD_DEFINITIONS("-std=c++0x")
# ADD_DEFINITIONS("-std=c++0x")

ADD_DEFINITIONS(
	${QT_DEFINITIONS}
	-fexceptions
)

PROJECT(qtsflphone)

SET(CMAKE_AUTOMOC TRUE)

#target_link_libraries(qtsflphone ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS})

SET(LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_MODULE_PATH "${LOCAL_CMAKE_MODULE_PATH}")

IF(NOT ${ENABLE_VIDEO} MATCHES false)
   MESSAGE("VIDEO enabled")
   SET(ENABLE_VIDEO 1 CACHE BOOLEAN "Enable video")
   add_definitions( -DENABLE_VIDEO=true )
ENDIF(NOT ${ENABLE_VIDEO} MATCHES false)

IF(${ENABLE_QT5} MATCHES true)
   FIND_PACKAGE(Qt5Core)
   FIND_PACKAGE(Qt5DBus)
   ADD_DEFINITIONS(-DQT_DISABLE_DEPRECATED_BEFORE=0)
ELSE()
   FIND_PACKAGE ( Qt4  REQUIRED )
ENDIF(${ENABLE_QT5} MATCHES true)

set(GENERIC_LIB_VERSION "1.4.1")

include_directories(SYSTEM ${QT_INCLUDES} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES ( ${CMAKE_CURRENT_BINARY_DIR})

# Build dbus interfaces
SET ( dbus_xml_introspecs_path ${CMAKE_CURRENT_SOURCE_DIR}/dbus/)

#File to compile
set( qtsflphone_LIB_SRCS
  #Data objects
  call.cpp
  uri.cpp
  account.cpp
  contact.cpp
  phonenumber.cpp
  numbercategory.cpp
  abstractitembackendmodelextension.cpp
  video/device.cpp
  video/renderer.cpp
  video/codec.cpp

  #Models
  accountmodel.cpp
  callmodel.cpp
  historymodel.cpp
  abstractbookmarkmodel.cpp
  credentialmodel.cpp
  instantmessagingmodel.cpp
  imconversationmanager.cpp
  contactproxymodel.cpp
  useractionmodel.cpp
  presencestatusmodel.cpp
  phonedirectorymodel.cpp
  historytimecategorymodel.cpp
  numbercategorymodel.cpp
  keyexchangemodel.cpp
  tlsmethodmodel.cpp
  numbercompletionmodel.cpp
  categorizedaccountmodel.cpp
  ringtonemodel.cpp
  lastusednumbermodel.cpp
  securityvalidationmodel.cpp
  certificate.cpp
  contactmodel.cpp
  itembackendmodel.cpp
  video/devicemodel.cpp
  video/sourcesmodel.cpp
  video/codecmodel2.cpp
  video/channel.cpp
  video/resolution.cpp
  video/rate.cpp
  video/manager.cpp
  audio/alsapluginmodel.cpp
  audio/codecmodel.cpp
  audio/inputdevicemodel.cpp
  audio/managermodel.cpp
  audio/outputdevicemodel.cpp
  audio/ringtonedevicemodel.cpp
  audio/settings.cpp


  #Data backends
  transitionalcontactbackend.cpp
  legacyhistorybackend.cpp
  abstractitembackend.cpp

  #Communication
  dbus/configurationmanager.cpp
  dbus/callmanager.cpp
  dbus/instancemanager.cpp
  dbus/videomanager.cpp
  dbus/presencemanager.cpp

  #Visitors
  visitors/accountlistcolorvisitor.cpp
  visitors/phonenumberselector.cpp
  visitors/numbercategoryvisitor.cpp
  visitors/pixmapmanipulationvisitor.cpp
  visitors/presenceserializationvisitor.cpp
  visitors/itemmodelstateserializationvisitor.cpp

  #Other
  sflphone_const.h
  categorizedcompositenode.cpp
  hookmanager.cpp

  #Extension
  extensions/presenceitembackendmodelextension.cpp
)

set( qtsflphone_LIB_HDRS
  account.h
  accountmodel.h
  call.h
  callmodel.h
  historymodel.h
  contact.h
  abstractitembackend.h
  abstractbookmarkmodel.h
  credentialmodel.h
  instantmessagingmodel.h
  imconversationmanager.h
  contactproxymodel.h
  useractionmodel.h
  presencestatusmodel.h
  phonenumber.h
  phonedirectorymodel.h
  historytimecategorymodel.h
  numbercategorymodel.h
  keyexchangemodel.h
  tlsmethodmodel.h
  numbercompletionmodel.h
  categorizedaccountmodel.h
  numbercategory.h
  ringtonemodel.h
  lastusednumbermodel.h
  securityvalidationmodel.h
  certificate.h
  contactmodel.h
  transitionalcontactbackend.h
  abstractitembackend.h
  itembackendmodel.h
  hookmanager.h
  uri.h
  abstractitembackendmodelextension.h
  video/device.h
  video/devicemodel.h
  video/sourcesmodel.h
  video/codec.h
  video/codecmodel2.h
  video/manager.h
  video/renderer.h
  video/resolution.h
  video/channel.h
  video/rate.h
  audio/alsapluginmodel.h
  audio/codecmodel.h
  audio/inputdevicemodel.h
  audio/managermodel.h
  audio/outputdevicemodel.h
  audio/ringtonedevicemodel.h
  audio/settings.h

  extensions/presenceitembackendmodelextension.h
  #commonbackendmanagerinterface.h
)

set( qtsflphone_extra_LIB_HDRS
  typedefs.h
  sflphone_const.h
  visitors/accountlistcolorvisitor.h
  visitors/phonenumberselector.h
  visitors/presenceserializationvisitor.h
  visitors/itemmodelstateserializationvisitor.h
)

# presence manager interface
SET ( presencemanager_xml  ${dbus_xml_introspecs_path}/presencemanager-introspec.xml )

SET_SOURCE_FILES_PROPERTIES(
   ${presencemanager_xml}
   PROPERTIES
   CLASSNAME PresenceManagerInterface
   INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/dbus/metatypes.h")

IF(${ENABLE_QT5} MATCHES true)
   QT5_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${presencemanager_xml}
      presencemanager_dbus_interface
   )
ELSE()
   QT4_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${presencemanager_xml}
      presencemanager_dbus_interface
   )

ENDIF(${ENABLE_QT5} MATCHES true)

# configuration manager interface
SET ( configurationmanager_xml  ${dbus_xml_introspecs_path}/configurationmanager-introspec.xml )

SET_SOURCE_FILES_PROPERTIES(
   ${configurationmanager_xml}
   PROPERTIES
   CLASSNAME ConfigurationManagerInterface
   INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/dbus/metatypes.h")

IF(${ENABLE_QT5} MATCHES true)
   QT5_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${configurationmanager_xml}
      configurationmanager_dbus_interface
   )
ELSE()
   QT4_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${configurationmanager_xml}
      configurationmanager_dbus_interface
   )

ENDIF(${ENABLE_QT5} MATCHES true)

# call manager interface
SET ( callmanager_xml  ${dbus_xml_introspecs_path}/callmanager-introspec.xml )

SET_SOURCE_FILES_PROPERTIES(
   ${callmanager_xml}
   PROPERTIES
   CLASSNAME CallManagerInterface
   INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/dbus/metatypes.h")

IF(${ENABLE_QT5} MATCHES true)
   QT5_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${callmanager_xml}
      callmanager_dbus_interface
   )
ELSE()
   QT4_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${callmanager_xml}
      callmanager_dbus_interface
   )
ENDIF(${ENABLE_QT5} MATCHES true)


# video manager interface
SET ( video_xml  ${dbus_xml_introspecs_path}/videomanager-introspec.xml )

SET_SOURCE_FILES_PROPERTIES(
   ${video_xml}
   PROPERTIES
   CLASSNAME VideoManagerInterface
   INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/dbus/metatypes.h")

IF(${ENABLE_QT5} MATCHES true)
   QT5_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${video_xml}
      video_dbus_interface
   )
ELSE()
   QT4_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${video_xml}
      video_dbus_interface
   )
ENDIF(${ENABLE_QT5} MATCHES true)


# instance interface
SET ( instance_xml  ${dbus_xml_introspecs_path}/instance-introspec.xml )

SET_SOURCE_FILES_PROPERTIES(
   ${instance_xml}
   PROPERTIES
   CLASSNAME InstanceInterface
   INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/dbus/metatypes.h")

IF(${ENABLE_QT5} MATCHES true)
   QT5_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${instance_xml}
      instance_dbus_interface
   )
ELSE()
   QT4_ADD_DBUS_INTERFACE(
      qtsflphone_LIB_SRCS
      ${instance_xml}
      instance_dbus_interface
   )
ENDIF(${ENABLE_QT5} MATCHES true)

# Manually wrap private files
SET(qtsflphone_PRIVATE_HDRS
   private/call_p.h
   private/phonedirectorymodel_p.h
   private/instantmessagingmodel_p.h
)

IF(${ENABLE_QT5} MATCHES true)
   QT5_WRAP_CPP(LIB_HEADER_MOC ${qtsflphone_PRIVATE_HDRS})
ELSE()
   QT4_WRAP_CPP(LIB_HEADER_MOC ${qtsflphone_PRIVATE_HDRS})
ENDIF(${ENABLE_QT5} MATCHES true)


add_library( qtsflphone  SHARED ${qtsflphone_LIB_SRCS} ${LIB_HEADER_MOC} )
# REMOVE_DEFINITIONS("-w")

IF(${ENABLE_QT5} MATCHES true)
   QT5_USE_MODULES(qtsflphone Core DBus)
ENDIF(${ENABLE_QT5} MATCHES true)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   SET(LINK_LIBRT "-lrt")
ENDIF()

target_link_libraries( qtsflphone
  ${LINK_LIBRT}
  -lpthread
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)

set_target_properties( qtsflphone
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION}
)

SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)

install( FILES ${qtsflphone_LIB_HDRS} ${qtsflphone_extra_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/qtsflphone
  COMPONENT Devel
)

#This hack force Debian based system to return a non multi-arch path
#this is required to prevent the .deb libqtsflphone.so from having an
#higher priority than the prefixed one.
STRING(REPLACE "${CMAKE_LIBRARY_ARCHITECTURE}" "" SANE_LIBRARY_PATH "${CMAKE_INSTALL_FULL_LIBDIR}" )

install( TARGETS qtsflphone
  ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
  LIBRARY DESTINATION ${SANE_LIBRARY_PATH}
  DESTINATION ${SANE_LIBRARY_PATH}
)
