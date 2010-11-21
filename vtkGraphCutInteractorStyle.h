#ifndef vtkGraphCutInteractorStyle_H
#define vtkGraphCutInteractorStyle_H

#include <vtkInteractorStyleImage.h> // superclass
#include <vtkSmartPointer.h>

#include "ImageGraphCut.h"

class vtkImageActor;
class vtkImageTracerWidget;
class vtkImageData;

class vtkGraphCutInteractorStyle : public vtkInteractorStyleImage
{
public:
  static vtkGraphCutInteractorStyle* New();
  vtkTypeMacro(vtkGraphCutInteractorStyle, vtkInteractorStyleImage);

  vtkGraphCutInteractorStyle();

  int GetSelectionType();
  enum SELECTION {FOREGROUND, BACKGROUND};

  void SetInteractionModeToForeground();
  void SetInteractionModeToBackground();

  vtkPolyData* GetForegroundSelection();
  vtkPolyData* GetBackgroundSelection();

  void ClearSelections();

  void InitializeTracer(vtkImageActor* imageActor);

private:
  void Refresh();
  void CatchWidgetEvent(vtkObject* caller, long unsigned int eventId, void* callData);

  int SelectionType;

  vtkSmartPointer<vtkImageTracerWidget> Tracer;
  vtkSmartPointer<vtkImageData> ResultImage;
  vtkSmartPointer<vtkPolyData> ForegroundSelection;
  vtkSmartPointer<vtkPolyData> BackgroundSelection;

  vtkSmartPointer<vtkPolyDataMapper> BackgroundSelectionMapper;
  vtkSmartPointer<vtkPolyDataMapper> ForegroundSelectionMapper;

  vtkSmartPointer<vtkActor> BackgroundSelectionActor;
  vtkSmartPointer<vtkActor> ForegroundSelectionActor;

};

#endif