# Install script for directory: /home/mzy/Code/MpRpc/thirdparty/symlog

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/symlog" TYPE FILE FILES
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/CurrentThread.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/Date.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/MuduoThread.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/TimeStamp.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/TimeZone.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/Types.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/copyable.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/base/noncopyable.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/AsyncLogging.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/FileUtils.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/FixedBuffer.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/LogFile.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/LogStream.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/Logging.h"
    "/home/mzy/Code/MpRpc/thirdparty/symlog/src/log/symlog.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mzy/Code/MpRpc/thirdparty/symlog/lib/libsymlog.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsymlog.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/mzy/Code/MpRpc/thirdparty/symlog/build/test/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/mzy/Code/MpRpc/thirdparty/symlog/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
