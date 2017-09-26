# Install script for directory: /data/pckr144/potestio/hadressmacs/adressmacs/src/gmxlib

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/data/pckr144/potestio/hadressmacs/debug_hadress")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libraries")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/lib/libgmx.so.6;/data/pckr144/potestio/hadressmacs/debug_hadress/lib/libgmx.so")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/lib" TYPE SHARED_LIBRARY FILES
    "/data/pckr144/potestio/hadressmacs/adressmacs/src/gmxlib/CMakeFiles/CMakeRelink.dir/libgmx.so.6"
    "/data/pckr144/potestio/hadressmacs/adressmacs/src/gmxlib/CMakeFiles/CMakeRelink.dir/libgmx.so"
    )
  FOREACH(file
      "$ENV{DESTDIR}/data/pckr144/potestio/hadressmacs/debug_hadress/lib/libgmx.so.6"
      "$ENV{DESTDIR}/data/pckr144/potestio/hadressmacs/debug_hadress/lib/libgmx.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "development")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/lib/pkgconfig/libgmx.pc")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/lib/pkgconfig" TYPE FILE RENAME "libgmx.pc" FILES "/data/pckr144/potestio/hadressmacs/adressmacs/src/gmxlib/libgmx.pc")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "development")

