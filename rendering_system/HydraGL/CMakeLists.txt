#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL RenderSystem build
project("HydraGL" CXX)

set(HEADER_FILES
  src/OgreGLContext.h
  src/OgreGLDefaultHardwareBufferManager.h
  src/OgreGLDepthBuffer.h
  src/OgreGLFBOMultiRenderTarget.h
  src/OgreGLFBORenderTexture.h
  src/OgreGLFrameBufferObject.h
  src/OgreGLGpuProgram.h
  src/OgreGLGpuProgramManager.h
  src/OgreGLHardwareBufferManager.h
  src/OgreGLHardwareIndexBuffer.h
  src/OgreGLHardwareOcclusionQuery.h
  src/OgreGLHardwarePixelBuffer.h
  src/OgreGLHardwareVertexBuffer.h
  src/OgreGLPBRenderTexture.h
  src/OgreGLPBuffer.h
  src/OgreGLPixelFormat.h
  src/OgreGLPlugin.h
  src/OgreGLPrerequisites.h
  src/OgreGLRenderSystem.h
  src/OgreGLRenderTexture.h
  src/OgreGLRenderToVertexBuffer.h
  src/OgreGLSupport.h
  src/OgreGLTexture.h
  src/OgreGLTextureManager.h
)


set(SOURCE_FILES
  src/OgreGLContext.cpp
  src/OgreGLDefaultHardwareBufferManager.cpp
  src/OgreGLDepthBuffer.cpp
  src/OgreGLEngineDll.cpp
  src/OgreGLFBOMultiRenderTarget.cpp
  src/OgreGLFBORenderTexture.cpp
  src/OgreGLFrameBufferObject.cpp
  src/OgreGLGpuProgram.cpp
  src/OgreGLGpuProgramManager.cpp
  src/OgreGLHardwareBufferManager.cpp
  src/OgreGLHardwareIndexBuffer.cpp
  src/OgreGLHardwareOcclusionQuery.cpp
  src/OgreGLHardwarePixelBuffer.cpp
  src/OgreGLHardwareVertexBuffer.cpp
  src/OgreGLPBRenderTexture.cpp
  src/OgreGLPixelFormat.cpp
  src/OgreGLPlugin.cpp
  src/OgreGLRenderSystem.cpp
  src/OgreGLRenderTexture.cpp
  src/OgreGLRenderToVertexBuffer.cpp
  src/OgreGLSupport.cpp
  src/OgreGLTexture.cpp
  src/OgreGLTextureManager.cpp
)

set(GLSL_FILES
  src/GLSL/include/OgreGLSLExtSupport.h
  src/GLSL/include/OgreGLSLGpuProgram.h
  src/GLSL/include/OgreGLSLLinkProgram.h
  src/GLSL/include/OgreGLSLLinkProgramManager.h
  src/GLSL/include/OgreGLSLPreprocessor.h
  src/GLSL/include/OgreGLSLProgramFactory.h
  src/GLSL/include/OgreGLSLProgram.h
  src/GLSL/src/OgreGLSLExtSupport.cpp
  src/GLSL/src/OgreGLSLGpuProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgramManager.cpp
  src/GLSL/src/OgreGLSLPreprocessor.cpp
  src/GLSL/src/OgreGLSLProgram.cpp
  src/GLSL/src/OgreGLSLProgramFactory.cpp
)

# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS 
    src/win32/OgreWin32Context.h
    src/win32/OgreWin32GLSupport.h
    src/win32/OgreWin32Prerequisites.h
    src/win32/OgreWin32RenderTexture.h
    src/win32/OgreWin32Window.h
    src/win32/OgreGLUtil.h
  )
  set(PLATFORM_SOURCES
    src/win32/OgreWin32GLSupport.cpp
    src/win32/OgreWin32RenderTexture.cpp
    src/win32/OgreWin32Window.cpp
    src/win32/OgreWin32Context.cpp
  )
  include_directories(src/win32)
  source_group(GLSL FILES ${GLSL_FILES})
  set(PLATFORM_HEADER_INSTALL "")
elseif (APPLE)
  set(PLATFORM_HEADERS
    src/OSX/OgreOSXCGLContext.h
    src/OSX/OgreOSXCarbonContext.h
    src/OSX/OgreOSXCarbonWindow.h
    src/OSX/OgreOSXCocoaContext.h
	src/OSX/OgreOSXCocoaView.h
    src/OSX/OgreOSXCocoaWindow.h
    src/OSX/OgreOSXCocoaWindowDelegate.h
    src/OSX/OgreOSXContext.h
    src/OSX/OgreOSXGLSupport.h
    src/OSX/OgreOSXRenderTexture.h
    src/OSX/OgreOSXWindow.h
    src/OSX/OgreGLUtil.h
  )
  set(PLATFORM_SOURCES
    src/OSX/OgreOSXCarbonContext.cpp
    src/OSX/OgreOSXCocoaContext.mm
    src/OSX/OgreOSXContext.cpp
    src/OSX/OgreOSXRenderTexture.cpp
    src/OSX/OgreOSXCGLContext.cpp
    src/OSX/OgreOSXCarbonWindow.cpp
	src/OSX/OgreOSXCocoaView.mm
    src/OSX/OgreOSXCocoaWindow.mm
    src/OSX/OgreOSXCocoaWindowDelegate.mm
    src/OSX/OgreOSXGLSupport.mm
    src/OSX/OgreOSXWindow.cpp
  )
  include_directories(src/OSX "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_LIBS ${Carbon_LIBRARY_FWK} ${Cocoa_LIBRARY_FWK})
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  set(PLATFORM_HEADERS
    src/GLX/OgreGLXContext.h
    src/GLX/OgreGLXGLSupport.h
    src/GLX/OgreGLXRenderTexture.h
    src/GLX/OgreGLXUtils.h
    src/GLX/OgreGLXWindow.h
    src/GLX/OgreGLUtil.h
  )
  set(PLATFORM_SOURCES
    src/GLX/OgreGLXContext.cpp
    src/GLX/OgreGLXGLSupport.cpp
    src/GLX/OgreGLXRenderTexture.cpp
    src/GLX/OgreGLXWindow.cpp
  )
  include_directories(src/GLX)
  set(PLATFORM_HEADER_INSTALL "GLX")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()


#include_directories(
#  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
#)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include
  ${OPENGL_INCLUDE_DIR}
)

add_library(HydraGL SHARED ${HEADER_FILES} ${SOURCE_FILES} ${GLSL_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})

add_definitions(-DGLEW_STATIC -DOGRE_GLPLUGIN_EXPORTS)

include_directories(${GLEW_SOURCE_DIR})

target_link_libraries(HydraGL
	${Ogre_LIBRARY} ${OPENGL_LIBRARIES} ${PLATFORM_LIBS} glew)

set_target_properties(HydraGL PROPERTIES DEBUG_POSTFIX _d)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Plugins can be installed in the application package
    set_target_properties(RenderSystem_GL
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

#ogre_config_plugin(RenderSystem_GL)
#install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GL)
#install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GL/${PLATFORM_HEADER_INSTALL})
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GL DESTINATION include/OGRE/RenderSystems/GL)
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL)
