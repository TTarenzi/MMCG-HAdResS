# Install script for directory: /data/pckr144/potestio/hadressmacs/adressmacs/scripts

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

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/GMXRC")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/GMXRC")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/GMXRC.bash")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/GMXRC.bash")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/GMXRC.zsh")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/GMXRC.zsh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/GMXRC.csh")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/GMXRC.csh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/completion.bash;/data/pckr144/potestio/hadressmacs/debug_hadress/bin/completion.csh;/data/pckr144/potestio/hadressmacs/debug_hadress/bin/completion.zsh")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE FILE FILES
    "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/completion.bash"
    "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/completion.csh"
    "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/completion.zsh"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/demux.pl")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/demux.pl")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/data/pckr144/potestio/hadressmacs/debug_hadress/bin/xplor2gmx.pl")
FILE(INSTALL DESTINATION "/data/pckr144/potestio/hadressmacs/debug_hadress/bin" TYPE PROGRAM FILES "/data/pckr144/potestio/hadressmacs/adressmacs/scripts/xplor2gmx.pl")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime")

