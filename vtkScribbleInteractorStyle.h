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

/*
 * This class is responsible for the user interaction with the input image.
 * A vtkImageTracerWidget does most of the work, but this class appends and maintains
 * the selections.
*/

#ifndef vtkScribbleInteractorStyle_H
#define vtkScribbleInteractorStyle_H

#include <vtkImageTracerWidget.h>
#include <vtkInteractorStyleImage.h> // superclass
#include <vtkSmartPointer.h>

#include "ImageGraphCut.h"

class vtkImageActor;
class vtkImageData;
class vtkPolyData;

class vtkScribbleInteractorStyle : public vtkInteractorStyleImage
{
public:
  static vtkScribbleInteractorStyle* New();
  vtkTypeMacro(vtkScribbleInteractorStyle, vtkInteractorStyleImage);

  vtkScribbleInteractorStyle();

  int GetSelectionType();
  enum SELECTION {FOREGROUND, BACKGROUND};

  void SetInteractionModeToForeground();
  void SetInteractionModeToBackground();

  std::vector<itk::Index<2> > GetForegroundSelection();
  std::vector<itk::Index<2> > GetBackgroundSelection();

  // Empty both the foreground and background selection
  void ClearSelections();

  // Connect the tracer to the interactor, etc.
  void InitializeTracer(vtkImageActor* imageActor);

private:
  void Refresh();

  // Update the selection when the EndInteraction event is fired.
  void CatchWidgetEvent(vtkObject* caller, long unsigned int eventId, void* callData);

  // The state (foreground or background) of the selection.
  int SelectionType;

  // The widget which does most of the work.
  vtkSmartPointer<vtkImageTracerWidget> Tracer;

  // Keep track of the pixels the user selected.
  std::vector<itk::Index<2> > ForegroundSelection;
  std::vector<itk::Index<2> > BackgroundSelection;

  // Data, mapper, and actor for the selections
  vtkSmartPointer<vtkPolyData> ForegroundSelectionPolyData;
  vtkSmartPointer<vtkPolyData> BackgroundSelectionPolyData;

  vtkSmartPointer<vtkPolyDataMapper> BackgroundSelectionMapper;
  vtkSmartPointer<vtkPolyDataMapper> ForegroundSelectionMapper;

  vtkSmartPointer<vtkActor> BackgroundSelectionActor;
  vtkSmartPointer<vtkActor> ForegroundSelectionActor;

};

// Helpers
std::vector<itk::Index<2> > PolyDataToPixelList(vtkPolyData* polydata);

#endif