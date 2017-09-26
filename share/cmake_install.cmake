# Install script for directory: /data/pckr144/potestio/hadressmacs/adressmacs/share

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

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "data")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/share/gromacs/.")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/share/gromacs" TYPE DIRECTORY FILES "/data/pckr144/potestio/hadressmacs/adressmacs/share/." REGEX "/Makefile[^/]*$" EXCLUDE REGEX "/CMake[^/]*$" EXCLUDE REGEX "/cmake[^/]*$" EXCLUDE REGEX "/template\\_doc\\.c$" EXCLUDE REGEX "/Template\\.mak$" EXCLUDE REGEX "/[^/]*\\~$" EXCLUDE)
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "data")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "data")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/share/gromacs/template/CMakeLists.txt")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/share/gromacs" TYPE FILE RENAME "template/CMakeLists.txt" FILES "/data/pckr144/potestio/hadressmacs/adressmacs/share/template/CMakeLists.txt.template")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "data")

