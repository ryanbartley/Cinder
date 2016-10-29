cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

set( CINDER_PLATFORM "Cocoa" )
set( CINDER_TARGET_SUBFOLDER "ios" )

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
add_definitions( -stdlib=libc++ )
set( IOS_PLATFORM "iPhoneOS" )
execute_process( COMMAND xcode-select --print-path OUTPUT_VARIABLE XCODE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE )
message( Xcode Path: ${XCODE_PATH} )
set( IOS_PLATFORM_DEVELOPER "${XCODE_PATH}/Platforms/${IOS_PLATFORM}.platform/Developer" )
message( Ios Path: ${IOS_PLATFORM_DEVELOPER} )
set( CMAKE_OSX_SYSROOT ${IOS_PLATFORM_DEVELOPER}/SDKs/iPhoneOS.sdk )

set( CMAKE_OSX_ARCHITECTURES "arm64" )
execute_process( COMMAND xcrun -find -sdk iphoneos clang OUTPUT_VARIABLE CLANG_PATH OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND xcrun -find -sdk iphoneos clang++ OUTPUT_VARIABLE CLANGXX_PATH OUTPUT_STRIP_TRAILING_WHITESPACE )
set( CMAKE_C_COMPILER ${CLANG_PATH} )
set( CMAKE_CXX_COMPILER ${CLANGXX_PATH} )

set( CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos" )

# message( FATAL_ERROR "Building with cmake for platform iOS not yet supported." )

# append mac specific source files
list( APPEND SRC_SET_COCOA
	${CINDER_SRC_DIR}/cinder/CaptureImplAvFoundation.mm
	${CINDER_SRC_DIR}/cinder/ImageSourceFileQuartz.cpp
	${CINDER_SRC_DIR}/cinder/ImageTargetFileQuartz.cpp
	${CINDER_SRC_DIR}/cinder/UrlImplCocoa.mm
	${CINDER_SRC_DIR}/cinder/cocoa/CinderCocoa.mm
)

list( APPEND SRC_SET_APP_COCOA
	${CINDER_SRC_DIR}/cinder/app/cocoa/AppCocoaTouch.cpp
	${CINDER_SRC_DIR}/cinder/app/cocoa/AppImplCocoaTouch.mm
	${CINDER_SRC_DIR}/cinder/app/cocoa/CinderViewCocoaTouch.mm
	${CINDER_SRC_DIR}/cinder/app/cocoa/PlatformCocoa.cpp
	${CINDER_SRC_DIR}/cinder/app/cocoa/RendererImpl2dCocoaTouchQuartz.mm
	${CINDER_SRC_DIR}/cinder/app/cocoa/RendererImplGlCocoaTouch.mm
)

list( APPEND SRC_SET_AUDIO_COCOA
	${CINDER_SRC_DIR}/cinder/audio/cocoa/CinderCoreAudio.cpp
	${CINDER_SRC_DIR}/cinder/audio/cocoa/ContextAudioUnit.cpp
	${CINDER_SRC_DIR}/cinder/audio/cocoa/FileCoreAudio.cpp
)

list( APPEND SRC_SET_QTIME
	${CINDER_SRC_DIR}/cinder/qtime/AvfUtils.mm
	${CINDER_SRC_DIR}/cinder/qtime/AvfWriter.mm
	${CINDER_SRC_DIR}/cinder/qtime/MovieWriter.cpp
	${CINDER_SRC_DIR}/cinder/qtime/QuickTimeGlImplAvf.cpp
	${CINDER_SRC_DIR}/cinder/qtime/QuickTimeImplAvf.mm
	${CINDER_SRC_DIR}/cinder/qtime/QuickTimeUtils.cpp
)

# specify what files need to be compiled as Objective-C++
list( APPEND CINDER_SOURCES_OBJCPP
	${CINDER_SRC_DIR}/cinder/Capture.cpp
	${CINDER_SRC_DIR}/cinder/Clipboard.cpp
	${CINDER_SRC_DIR}/cinder/Display.cpp
	${CINDER_SRC_DIR}/cinder/Font.cpp
	${CINDER_SRC_DIR}/cinder/Log.cpp
	${CINDER_SRC_DIR}/cinder/System.cpp
	${CINDER_SRC_DIR}/cinder/Utilities.cpp
	${CINDER_SRC_DIR}/cinder/app/AppBase.cpp
	${CINDER_SRC_DIR}/cinder/app/Renderer.cpp
	${CINDER_SRC_DIR}/cinder/app/RendererGl.cpp
	${CINDER_SRC_DIR}/cinder/app/Window.cpp
	${CINDER_SRC_DIR}/cinder/app/cocoa/AppCocoaTouch.cpp
	${CINDER_SRC_DIR}/cinder/app/cocoa/PlatformCocoa.cpp
	${CINDER_SRC_DIR}/cinder/gl/Environment.cpp
	${CINDER_SRC_DIR}/cinder/qtime/QuickTimeGlImplAvf.cpp

	${CINDER_SRC_DIR}/AntTweakBar/TwColors.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwFonts.cpp
	${CINDER_SRC_DIR}/AntTweakBar/LoadOGL.cpp
	${CINDER_SRC_DIR}/AntTweakBar/LoadOGLCore.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwBar.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwMgr.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwOpenGl.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwOpenGLCore.cpp
	${CINDER_SRC_DIR}/AntTweakBar/TwPrecomp.cpp
)

set_source_files_properties( ${CINDER_SOURCES_OBJCPP}
	PROPERTIES COMPILE_FLAGS "-x objective-c++"
)

list( APPEND CINDER_SRC_FILES
	${SRC_SET_COCOA}
	${SRC_SET_APP_COCOA}
	${SRC_SET_AUDIO_COCOA}
	${SRC_SET_QTIME}
)

list( APPEND CINDER_LIBS_DEPENDS
    ${CINDER_PATH}/lib/${CINDER_TARGET_SUBFOLDER}/libboost_system.a
    ${CINDER_PATH}/lib/${CINDER_TARGET_SUBFOLDER}/libboost_filesystem.a
)

set( CMAKE_SYSTEM_FRAMEWORK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks" )

# link in system frameworks
find_library( AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED )
find_library( AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED )
find_library( COREAUDIO_FRAMEWORK CoreAudio REQUIRED )
find_library( COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED )
find_library( COREMEDIA_FRAMEWORK CoreMedia REQUIRED )
find_library( CORETEXT_FRAMEWORK CoreText REQUIRED )
find_library( COREVIDEO_FRAMEWORK CoreVideo REQUIRED )
find_library( FOUNDATION_FRAMEWORK Foundation REQUIRED )
find_library( IMAGEIO_FRAMEWORK ImageIO REQUIRED )
find_library( MOBILECORESERVICES_FRAMEWORK MobileCoreServices REQUIRED )
find_library( OPENGLES_FRAMEWORK OpenGLES REQUIRED )
find_library( QUARTZCORE_FRAMEWORK QuartzCore REQUIRED )
find_library( UIKIT_FRAMEWORK UIKit REQUIRED )

list( APPEND CINDER_LIBS_DEPENDS
	${AUDIOTOOLBOX_FRAMEWORK} 
	${AVFOUNDATION_FRAMEWORK}
	${COREAUDIO_FRAMEWORK}
	${COREGRAPHICS_FRAMEWORK}
	${COREMEDIA_FRAMEWORK}
	${CORETEXT_FRAMEWORK}
	${COREVIDEO_FRAMEWORK}
	${FOUNDATION_FRAMEWORK}
	${IMAGEIO_FRAMEWORK}
	${MOBILECORESERVICES_FRAMEWORK}
	${OPENGLES_FRAMEWORK}
	${QUARTZCORE_FRAMEWORK}
	${UIKIT_FRAMEWORK}
)

source_group( "cinder\\cocoa"           FILES ${SRC_SET_COCOA} )
source_group( "cinder\\app\\cocoa"      FILES ${SRC_SET_APP_COCOA} )
source_group( "cinder\\audio\\cocoa"    FILES ${SRC_SET_AUDIO_COCOA} )

# These are samples that cannot be built on Mac OS X, indicating they should be skipped with CINDER_BUILD_SAMPLES is on.
list( APPEND CINDER_SKIP_SAMPLES
	_opengl/ParticleSphereCS
	_opengl/NVidiaComputeParticles
)

