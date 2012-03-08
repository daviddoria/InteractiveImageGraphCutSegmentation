/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Instantiate and display the GUI

#include <QApplication>

#include <iostream>

#include "GraphCutSegmentationWidget.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  GraphCutSegmentationWidget* graphCutSegmentationWidget = NULL;
  if(argc == 1)
  {
    graphCutSegmentationWidget = new GraphCutSegmentationWidget;
  }
  else if(argc == 2)
  {
    std::string fileName = argv[1];
    std::cout << "filename: " << fileName << std::endl;
    graphCutSegmentationWidget = new GraphCutSegmentationWidget(fileName);
  }
  
  graphCutSegmentationWidget->show();
  return app.exec();
}