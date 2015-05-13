#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software. 
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

# borrowed liberally from ParaView superbuild

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

#FUNCTION(generate_cmake_info SOURCE_DIR INSTALL_DIR)
#ENDFUNCTION()
#
#IF(WIN32)
#  SET(python_LIB_PREFIX "")
#ELSE()
#  SET(python_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
#ENDIF()

#SET(python_LIBRARY_PATH "${python_BINARY_DIR}/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
# other configuration options are RELWITHDEBINFO and MINSIZEREL
##SET(python_LIBRARY_PATH_RELEASE "${python_BINARY_DIR}/Release/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
##SET(python_LIBRARY_PATH_DEBUG "${python_BINARY_DIR}/Debug/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# TODO: update when upgrading
SET(PY_MAJOR 3)
SET(PY_MINOR 3)
SET(PY_PATCH 6)
SET(SCI_PYTHON_VERSION "${PY_MAJOR}.${PY_MINOR}.${PY_PATCH}")
SET(SCI_PYTHON_VERSION_SHORT "${PY_MAJOR}.${PY_MINOR}")
SET(SCI_PYTHON_VERSION_SHORT_WIN32 "${PY_MAJOR}${PY_MINOR}")

# TODO: recheck when upgrading
# --with-pydebug
#SET(python_ABIFLAG_PYDEBUG "d")
SET(python_ABIFLAG_PYDEBUG)
# --with-pymalloc (default)
# if disabling pymalloc (--without-pymalloc) for valgrind or to track other memory problems,
# disable this ABI flag
SET(python_ABIFLAG_PYMALLOC "m")
SET(ABIFLAGS "${python_ABIFLAG_PYMALLOC}${python_ABIFLAG_PYDEBUG}")

SET(python_GIT_TAG "origin/python_3.3.6")
SET(python_GIT_URL "https://github.com/CIBC-Internal/python.git")

SET(python_WIN32_ARCH)
SET(python_WIN32_64BIT_DIR)
SET(python_FRAMEWORK_ARCHIVE)

IF(UNIX)
  SET(python_CONFIGURE_FLAGS
    "--prefix=<INSTALL_DIR>"
#    "--exec-prefix=<INSTALL_DIR>"
    "--with-threads"
  )
  IF(APPLE)
    # framework contains *.dylib
    LIST(APPEND python_CONFIGURE_FLAGS "--enable-framework=<INSTALL_DIR>")
    SET(python_FRAMEWORK_ARCHIVE "framework.tar")
  ELSE()
    LIST(APPEND python_CONFIGURE_FLAGS "--enable-shared")
  ENDIF()
ELSE()
  # TODO: 32-bit windows build?
  SET(python_WIN32_ARCH "x64")
  # 64-bit build only
  # 32-bit build outputs to PCbuild dir
  SET(python_WIN32_64BIT_DIR "/amd64")
  #SET(python_ABIFLAG_PYDEBUG "_d")
ENDIF()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
IF(UNIX)
  ExternalProject_Add(Python_external
    GIT_REPOSITORY ${python_GIT_URL}
    GIT_TAG ${python_GIT_TAG}
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${python_CONFIGURE_FLAGS}
    PATCH_COMMAND ""
  )
  IF(APPLE)
    # Preserves links, permissions
    ExternalProject_Add_Step(Python_external framework_tar_archive
      COMMAND "${CMAKE_COMMAND}" -E tar cvf ${python_FRAMEWORK_ARCHIVE} Python.framework
	DEPENDEES install
	WORKING_DIRECTORY <INSTALL_DIR>
    )
  ENDIF()
ELSE()
  ExternalProject_Add(Python_external
    GIT_REPOSITORY ${python_GIT_URL}
    GIT_TAG ${python_GIT_TAG}
	BUILD_IN_SOURCE ON
	CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo /property:Configuration=Release /property:Platform=${python_WIN32_ARCH}
    PATCH_COMMAND ""
    INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      <SOURCE_DIR>/PC/pyconfig.h
      <SOURCE_DIR>/Include/pyconfig.h
  )
  # build both Release and Debug versions
  ExternalProject_Add_Step(Python_external debug_build
    COMMAND ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo /property:Configuration=Debug /property:Platform=${python_WIN32_ARCH}
      DEPENDEES build
      DEPENDERS install
      WORKING_DIRECTORY <SOURCE_DIR>
  )
ENDIF()

ExternalProject_Get_Property(Python_external SOURCE_DIR)
ExternalProject_Get_Property(Python_external INSTALL_DIR)

IF(UNIX)
  IF(APPLE)
    SET(SCI_PYTHON_FRAMEWORK ${INSTALL_DIR}/Python.framework)
    SET(SCI_PYTHON_ROOT_DIR ${SCI_PYTHON_FRAMEWORK}/Versions/${SCI_PYTHON_VERSION_SHORT})
    SET(SCI_PYTHON_INCLUDE ${SCI_PYTHON_ROOT_DIR}/Headers)
    SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}/lib)
    SET(SCI_PYTHON_EXE ${SCI_PYTHON_ROOT_DIR}/bin/python${SCI_PYTHON_VERSION_SHORT})
    SET(SCI_PYTHON_LIBRARY python${SCI_PYTHON_VERSION_SHORT})

    # required by interpreter interface
    SET(PYTHON_MODULE_SEARCH_PATH ../Frameworks/Python.framework/Versions/${SCI_PYTHON_VERSION_SHORT}/lib/python${SCI_PYTHON_VERSION_SHORT} CACHE INTERNAL "Python modules." FORCE)
    SET(SCI_PYTHON_FRAMEWORK_ARCHIVE ${INSTALL_DIR}/${python_FRAMEWORK_ARCHIVE})
  ELSE()
    SET(SCI_PYTHON_ROOT_DIR ${INSTALL_DIR})
    SET(SCI_PYTHON_INCLUDE ${INSTALL_DIR}/include/python${SCI_PYTHON_VERSION_SHORT}${ABIFLAGS})
    IF(SEG3D_BITS MATCHES 64)
      SET(SCI_PYTHON_LIBRARY_DIR ${INSTALL_DIR}/lib64)
    ELSE()
      SET(SCI_PYTHON_LIBRARY_DIR ${INSTALL_DIR}/lib)
    ENDIF()
    SET(SCI_PYTHON_EXE ${INSTALL_DIR}/bin/python${SCI_PYTHON_VERSION_SHORT})
    SET(SCI_PYTHON_LIBRARY python${SCI_PYTHON_VERSION_SHORT}${ABIFLAGS})

    # required by interpreter interface
    SET(PYTHON_MODULE_SEARCH_PATH lib/python${SCI_PYTHON_VERSION_SHORT} CACHE INTERNAL "Python modules." FORCE)
    SET(SCI_PYTHON_MODULE_LIBRARY_PATH ${INSTALL_DIR}/lib/python${SCI_PYTHON_VERSION_SHORT})
  ENDIF()
ELSE()
  # Windows does not do install step
  SET(SCI_PYTHON_ROOT_DIR ${SOURCE_DIR}/PCbuild)
  SET(SCI_PYTHON_INCLUDE ${SOURCE_DIR}/Include)
  SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR})
  # 32bit executable name?
  SET(SCI_PYTHON_EXE ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/python.exe)
  # release build
  # debug build has _d suffix
  SET(SCI_PYTHON_LIBRARY python${SCI_PYTHON_VERSION_SHORT_WIN32})

  # required by interpreter interface
  SET(PYTHON_MODULE_SEARCH_PATH lib/python${SCI_PYTHON_VERSION_SHORT} CACHE INTERNAL "Python modules." FORCE)
  SET(SCI_PYTHON_MODULE_LIBRARY_PATH ${SOURCE_DIR}/Lib)
ENDIF()

SET(SCI_PYTHON_LIBRARY_PATH ${SCI_PYTHON_LIBRARY_DIR}/${SCI_PYTHON_LIBRARY}${CMAKE_SHARED_LIBRARY_SUFFIX})
SET(SCI_PYTHON_USE_FILE ${INSTALL_DIR}/UsePython.cmake)

# Python is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/PythonConfig.cmake.in ${INSTALL_DIR}/PythonConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UsePython.cmake ${SCI_PYTHON_USE_FILE} COPYONLY)

SET(Python_DIR ${INSTALL_DIR} CACHE PATH "")

MESSAGE(STATUS "Python_DIR: ${Python_DIR}")
