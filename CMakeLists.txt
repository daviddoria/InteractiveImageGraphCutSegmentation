cmake_minimum_required(VERSION 2.6)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

Project(InteractiveImageGraphCutSegmentation)

# We need to compile with c++11 functionality since it is
# used by the Helpers subsubmodule.
if(UNIX)
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x") # gcc < 4.7
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11") # gcc >= 4.7
endif(UNIX)

# Turn off warnings seen in VS2010 in ITK code.
if(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")
endif(WIN32)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${QT_LIBRARIES})

QT4_WRAP_UI(GraphCutSegmentationUISrcs GraphCutSegmentationWidget.ui)
QT4_WRAP_CPP(GraphCutSegmentationMOCSrcs GraphCutSegmentationWidget.h)

# Allow Qt to find it's MOCed files
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${USE_VTK_FILE} )

if( "${VTK_MAJOR_VERSION}" LESS 6 )
  MESSAGE(FATAL_ERROR "You must build this code with VTK >= 6.0!")
endif( "${VTK_MAJOR_VERSION}" LESS 6 )

set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${VTK_LIBRARIES})

# ITK
FIND_PACKAGE(ITK REQUIRED)
INCLUDE( ${USE_ITK_FILE} )

if( "${ITK_VERSION_MAJOR}" LESS 4 )
  MESSAGE(FATAL_ERROR "You must build this code with ITK >= 4.0!")
endif( "${ITK_VERSION_MAJOR}" LESS 4 )

set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${ITK_LIBRARIES})

# Submodules
# Mask
get_property(FoundMask GLOBAL PROPERTY MaskIncludeDirs SET)
if(NOT FoundMask)
  add_subdirectory(Mask)
endif()

get_property(MaskIncludeDirs GLOBAL PROPERTY MaskIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${MaskIncludeDirs})
get_property(MaskLibs GLOBAL PROPERTY MaskLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${MaskLibs})

# ITKVTKHelpers
get_property(FoundITKVTKHelpers GLOBAL PROPERTY ITKVTKHelpersIncludeDirs SET)
if(NOT FoundITKVTKHelpers)
  add_subdirectory(ITKVTKHelpers)
endif()

get_property(ITKVTKHelpersIncludeDirs GLOBAL PROPERTY ITKVTKHelpersIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${ITKVTKHelpersIncludeDirs})
get_property(ITKVTKHelpersLibs GLOBAL PROPERTY ITKVTKHelpersLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${ITKVTKHelpersLibs})

# ScribbleInteractorStyle
get_property(FoundScribbleInteractorStyle GLOBAL PROPERTY ScribbleInteractorStyleIncludeDirs SET)
if(NOT FoundScribbleInteractorStyle)
  add_subdirectory(ScribbleInteractorStyle)
endif()

get_property(ScribbleInteractorStyleIncludeDirs GLOBAL PROPERTY ScribbleInteractorStyleIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${ScribbleInteractorStyleIncludeDirs})
get_property(ScribbleInteractorStyleLibs GLOBAL PROPERTY ScribbleInteractorStyleLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${ScribbleInteractorStyleLibs})

# VTKHelpers
get_property(FoundVTKHelpers GLOBAL PROPERTY VTKHelpersIncludeDirs SET)
if(NOT FoundVTKHelpers)
  add_subdirectory(VTKHelpers)
endif()

get_property(VTKHelpersIncludeDirs GLOBAL PROPERTY VTKHelpersIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${VTKHelpersIncludeDirs})
get_property(VTKHelpersLibs GLOBAL PROPERTY VTKHelpersLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${VTKHelpersLibs})

# ITKVTKCamera
get_property(FoundITKVTKCamera GLOBAL PROPERTY ITKVTKCameraIncludeDirs SET)
if(NOT FoundITKVTKCamera)
  add_subdirectory(ITKVTKCamera)
endif()

get_property(ITKVTKCameraIncludeDirs GLOBAL PROPERTY ITKVTKCameraIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${ITKVTKCameraIncludeDirs})
get_property(ITKVTKCameraLibs GLOBAL PROPERTY ITKVTKCameraLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${ITKVTKCameraLibs})

# ImageGraphCutSegmentation
get_property(FoundImageGraphCutSegmentation GLOBAL PROPERTY ImageGraphCutSegmentationIncludeDirs SET)
if(NOT FoundImageGraphCutSegmentation)
  add_subdirectory(ImageGraphCutSegmentation)
endif()

get_property(ImageGraphCutSegmentationIncludeDirs GLOBAL PROPERTY ImageGraphCutSegmentationIncludeDirs)
set(InteractiveImageGraphCutSegmentation_include_dirs ${InteractiveImageGraphCutSegmentation_include_dirs} ${ImageGraphCutSegmentationIncludeDirs})
get_property(ImageGraphCutSegmentationLibs GLOBAL PROPERTY ImageGraphCutSegmentationLibs)
set(InteractiveImageGraphCutSegmentation_libraries ${InteractiveImageGraphCutSegmentation_libraries} ${ImageGraphCutSegmentationLibs})

# Give the compiler all of the required include directories
include_directories(${InteractiveImageGraphCutSegmentation_include_dirs})

# Build the executable
ADD_EXECUTABLE(InteractiveImageGraphCutSegmentation
InteractiveGraphCutSegmentation.cpp GraphCutSegmentationWidget.cpp
${GraphCutSegmentationMOCSrcs} ${GraphCutSegmentationUISrcs})
TARGET_LINK_LIBRARIES(InteractiveImageGraphCutSegmentation
${InteractiveImageGraphCutSegmentation_libraries}
)

