cmake_minimum_required(VERSION 2.6)

Project(InteractiveImageGraphCutSegmentation)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UISrcs MainWindow.ui)
QT4_WRAP_CPP(MOCSrcs MainWindow.h ProgressThread.h)

#include_directories(${include_directories} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${USE_VTK_FILE} )

FIND_PACKAGE(ITK REQUIRED)
INCLUDE( ${USE_ITK_FILE} )

ADD_EXECUTABLE(InteractiveImageGraphCutSegmentation main.cpp MainWindow.cpp
graph.cpp maxflow.cpp Helpers.cpp
vtkScribbleInteractorStyle.cxx
ImageGraphCut.cxx
${MOCSrcs} ${UISrcs})
TARGET_LINK_LIBRARIES(InteractiveImageGraphCutSegmentation vtkHybrid vtkWidgets
QVTK ${QT_LIBRARIES} QtXml
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
