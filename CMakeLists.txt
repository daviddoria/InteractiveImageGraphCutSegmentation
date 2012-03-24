cmake_minimum_required(VERSION 2.6)

Project(InteractiveImageGraphCutSegmentation)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(GraphCutSegmentationUISrcs GraphCutSegmentationWidget.ui)
QT4_WRAP_CPP(GraphCutSegmentationMOCSrcs GraphCutSegmentationWidget.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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

add_library(MaxFlow graph.cpp maxflow.cpp)

ADD_EXECUTABLE(InteractiveImageGraphCutSegmentation
InteractiveGraphCutSegmentation.cpp GraphCutSegmentationWidget.cpp
Helpers.cpp
vtkScribbleInteractorStyle.cxx
ImageGraphCut.cxx
${GraphCutSegmentationMOCSrcs} ${GraphCutSegmentationUISrcs})
TARGET_LINK_LIBRARIES(InteractiveImageGraphCutSegmentation ${VTK_LIBRARIES}
QVTK ${QT_LIBRARIES} QtXml MaxFlow
${ITK_LIBRARIES}
)
#
# ADD_EXECUTABLE(NonInteractive NonInteractive.cpp
# graph.cpp maxflow.cpp
# Helpers.cpp
# )
# TARGET_LINK_LIBRARIES(NonInteractive vtkHybrid vtkWidgets
# ITKCommon ITKIO ITKStatistics
# )
