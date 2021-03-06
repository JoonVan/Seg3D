#
#  For more information, please see: http://software.sci.utah.edu
#  The MIT License
#
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
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
#


############################################
# Set Seg3D main sources
###########################################

SET(MAIN_SRCS
  main.cc
  Seg3DBase.h
  Seg3DBase.cc
  Seg3DGui.h
  Seg3DGui.cc
  Seg3DHeadless.h
  Seg3DHeadless.cc
)

###########################################
# Build the Seg3D executable
###########################################

IF(NOT BUILD_STANDALONE_LIBRARY)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})
  ADD_DEFINITIONS(-DAPPLICATION_NAME=${APPLICATION_NAME})
  IF(APPLE)
    LIST(APPEND MAIN_SRCS ${APPLICATION_OSX_ICON_FULL_PATH})
    SET_SOURCE_FILES_PROPERTIES(${APPLICATION_OSX_ICON_FULL_PATH}
      PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
    ADD_EXECUTABLE(${APPLICATION_NAME} MACOSX_BUNDLE ${MAIN_SRCS})
    SET_TARGET_PROPERTIES(${APPLICATION_NAME}
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${SEG3D_SOURCE_DIR}/Resources/info.plist
      MACOSX_BUNDLE_ICON_FILE ${APPLICATION_OSX_ICON}
    )
  ELSE()
    IF(WIN32)
      LIST(APPEND MAIN_SRCS ${APPLICATION_WINDOWS_RC})
      IF(SEG3D_SHOW_CONSOLE)
        ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
      ELSE()
        ADD_EXECUTABLE(${APPLICATION_NAME} WIN32 ${MAIN_SRCS})
      ENDIF()
    ELSE()
      ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
    ENDIF()
  ENDIF()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  ${SCI_BOOST_LIBRARY}
  Core_Utils
  Core_DataBlock
  Core_EventHandler
  Core_Graphics
  Core_Volume
  Core_Application
  Core_Interface
  Core_Action
  Core_Log
  Application_Tools
  Application_Filters
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  Interface_Application
  Interface_ToolInterface
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${ImplicitFunction_LIBRARY}
  ${RESOURCES_LIBRARY}
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} ${Qt5Core_QTMAIN_LIBRARIES} )
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
ENDIF()

IF(APPLE)
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "-framework CoreGraphics"
  )
ENDIF()

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
    Core_Python
    Application_Socket
  )
ENDIF()

# add libraries with plug-ins and extensions
REGISTERED_TARGET_LINK_LIBRARIES(${APPLICATION_NAME})

###########################################
# SETUP MACOSX APPLICATION BUNDLE
# There is no good way for incorporating
# external libraries into a bundle, hence
# it is hard coded here what needs to be
# done to generate an application that
# does not depend on anything except the
# operating system
###########################################

IF(APPLE)
  # set up app bundle
  ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
    POST_BUILD
    # configure Qt libs
    COMMAND ${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=${MACDEPLOYQT_OUTPUT_LEVEL}
    WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  IF(BUILD_WITH_PYTHON)
    SET(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
    SET(FRAMEWORK_PATH Frameworks/Python.framework)
    SET(PYTHON_PATH ${APPLICATION_NAME}.app/Contents/${FRAMEWORK_PATH}/${VERSION_PATH})
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      # macdeployqt puts frameworks in app/Content/Frameworks
      COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${APPLICATION_NAME}.app/Contents/Frameworks tar xf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
      COMMAND chmod u+w  ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
      # TODO: other executables...
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../Python ${PYTHON_PATH}/bin/python${SCI_PYTHON_VERSION_SHORT}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  ENDIF()
ELSEIF(WIN32)
  MESSAGE(STATUS "Setting ${WINDEPLOYQT} Command for path ${SEG3D_BINARY_DIR}/$<CONFIG>")
  ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
  POST_BUILD
  # copy Qt libs
  COMMAND ${WINDEPLOYQT} ARGS --verbose 5 -core -widgets -gui -opengl --libdir ${SEG3D_BINARY_DIR}/$<CONFIG> ${SEG3D_BINARY_DIR}/$<CONFIG>/${APPLICATION_NAME}.exe
  )

  IF(BUILD_WITH_PYTHON)
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
    # TODO: copy python dll
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<CONFIG>/${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} $<CONFIG>/${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  ENDIF()
ELSE()
  IF(BUILD_WITH_PYTHON)
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    IF(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH AND EXISTS ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH})
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory  ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
        WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    ENDIF()
  ENDIF()
ENDIF()

IF(APPLE)
  INSTALL(TARGETS ${APPLICATION_NAME}
        RUNTIME DESTINATION .
        BUNDLE DESTINATION .
  )
ELSE()
  GET_FILENAME_COMPONENT(QTCORENAME ${QtCore_location_Release} NAME_WE)
  GET_FILENAME_COMPONENT(QTGUINAME ${QtGui_location_Release} NAME_WE)
  GET_FILENAME_COMPONENT(QTOPENGLNAME ${QtOpenGL_location_Release} NAME_WE)
  GET_FILENAME_COMPONENT(QTWIDGETSNAME ${QtWidgets_location_Release} NAME_WE)
  GET_FILENAME_COMPONENT(QTSVGNAME ${QtSvg_location_Release} NAME_WE)

  IF(WIN32)
    INSTALL(TARGETS ${APPLICATION_NAME}
      RUNTIME DESTINATION bin
    )

    INSTALL(PROGRAMS
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTCORENAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTGUINAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTOPENGLNAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTWIDGETSNAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTSVGNAME}.dll
        DESTINATION bin
    )

    INSTALL(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/platforms/
        DESTINATION bin/platforms
    )

    INSTALL(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/translations/
        DESTINATION bin/translations
    )

    INSTALL(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/imageformats/
        DESTINATION bin/imageformats
    )

    INSTALL(FILES
      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
        DESTINATION bin
    )

    IF(BUILD_WITH_PYTHON)
      INSTALL(PROGRAMS
        ${SCI_PYTHON_DLL_PATH}
          DESTINATION bin
      )

      INSTALL(DIRECTORY
        "${SCI_PYTHON_MODULE_LIBRARY_PATH}/"
          DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
      )
    ENDIF()
#  ELSE()
#    INSTALL(TARGETS ${APPLICATION_NAME}
#      RUNTIME DESTINATION bin
#    )
#
#    INSTALL(FILES
#      ${QT_QTCORE_LIBRARY_RELEASE}.${QT_VERSION}
#      ${QT_QTGUI_LIBRARY_RELEASE}.${QT_VERSION}
#      ${QT_QTOPENGL_LIBRARY_RELEASE}.${QT_VERSION}
#      DESTINATION lib
#    )
#
#    INSTALL(FILES
#      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
#      DESTINATION bin
#    )
#
#    IF(BUILD_WITH_PYTHON)
#      INSTALL(FILES
#        ${SCI_PYTHON_DLL_PATH}
#        DESTINATION bin
#      )
#      INSTALL(DIRECTORY
#        ${SCI_PYTHON_MODULE_LIBRARY_PATH}
#        DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
#      )
#    ENDIF()
  ENDIF()
ENDIF()
ENDIF()

IF(BUILD_STANDALONE_LIBRARY)
  ADD_SUBDIRECTORY(Seg3DLib)
ENDIF()
