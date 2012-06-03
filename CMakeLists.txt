cmake_minimum_required(VERSION 2.6)

Project(InteractiveImageGraphCutSegmentation)

# We need to compile with c++11 functionality since it is
# used by the Helpers subsubmodule.
if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif(UNIX)

# Turn off warnings seen in VS2010 in ITK code.
if(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")
endif(WIN32)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(GraphCutSegmentationUISrcs GraphCutSegmentationWidget.ui)
QT4_WRAP_CPP(GraphCutSegmentationMOCSrcs GraphCutSegmentationWidget.h)

# Allow Qt to find it's MOCed files
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Submodules
add_subdirectory(Mask)
add_subdirectory(ITKVTKHelpers)
add_subdirectory(ScribbleInteractorStyle)
add_subdirectory(VTKHelpers)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${USE_VTK_FILE} )

if( "${VTK_MAJOR_VERSION}" LESS 6 )
  MESSAGE(FATAL_ERROR "You must build this code with VTK >= 6.0!")
endif( "${VTK_MAJOR_VERSION}" LESS 6 )

FIND_PACKAGE(ITK REQUIRED)
INCLUDE( ${USE_ITK_FILE} )

if( "${ITK_VERSION_MAJOR}" LESS 4 )
  MESSAGE(FATAL_ERROR "You must build this code with ITK >= 4.0!")
endif( "${ITK_VERSION_MAJOR}" LESS 4 )

add_library(KolmogorovMaxFlow graph.cpp maxflow.cpp)

add_library(ImageGraphCut ImageGraphCut.cxx)

ADD_EXECUTABLE(NonInteractive NonInteractive.cpp)
TARGET_LINK_LIBRARIES(NonInteractive
ImageGraphCut
KolmogorovMaxFlow
${VTK_LIBRARIES} ${ITK_LIBRARIES}
VTKHelpers ITKVTKHelpers Mask)

ADD_EXECUTABLE(InteractiveImageGraphCutSegmentation
InteractiveGraphCutSegmentation.cpp GraphCutSegmentationWidget.cpp
${GraphCutSegmentationMOCSrcs} ${GraphCutSegmentationUISrcs})
TARGET_LINK_LIBRARIES(InteractiveImageGraphCutSegmentation
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES}
ImageGraphCut
KolmogorovMaxFlow
VTKHelpers ITKVTKHelpers Mask Scribble
)

