This software allows the user to "scribble" on the foreground and background of an image to seed a graph cuts based segmentation.
This implementation is based on "Graph Cuts and Efficient N-D Image Segmentation" by Yuri Boykov (IJCV 2006).

Licesnse: GPLv3 (See LICENSE.txt)

Installation notes:
- You must use the git version of VTK from at least December 01, 2010 due to the use of recent changes in vtkImageTracerWidget.
- You must turn on ITK_USE_REVIEW in the ITK build because of the use of the new histogram/statistics classes
- This software was developed using Qt 4.7.1