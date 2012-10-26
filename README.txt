Getting the code
----------------
After you have cloned this repository, you will need to initialize the submodules:
git submodule update --init --recursive

If you ever update with 'git pull origin master', you must then do 'git submodule update --recursive'.

This pulls in all of the dependencies including Mask (which includes ITKHelpers and then Helpers), ITKVTKHelpers,
VTKHelpers, and ScribbleInteractorStyle.

Overview
--------
This software allows the user to "scribble" on the foreground and background of an image to seed a graph cuts based segmentation.
This implementation is based on "Graph Cuts and Efficient N-D Image Segmentation" by Yuri Boykov (IJCV 2006).

License
--------
GPLv3 (See LICENSE.txt). This is required because of the use of Kolmogorov's code.

Build notes
------------------
This repository does not depend on any external libraries. The only caveat is that it depends
on c++0x/11 parts of the c++ language. For Linux, this means it must be built with the flag
gnu++0x. For Windows, we are working on finding the comparable solution/flag.

Dependencies
------------
- VTK >= 6
- ITK >= 4
- Qt >= 4.7.1
