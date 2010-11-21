#include "vtkGraphCutInteractorStyle.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageTracerWidget.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkGraphCutInteractorStyle);

vtkGraphCutInteractorStyle::vtkGraphCutInteractorStyle()
{
  // Initializations
  this->Tracer = vtkSmartPointer<vtkImageTracerWidget>::New();
  this->Tracer->GetLineProperty()->SetLineWidth(5);
  this->ResultImage = vtkSmartPointer<vtkImageData>::New();

  // Foreground
  this->ForegroundSelection = vtkSmartPointer<vtkPolyData>::New();
  this->ForegroundSelectionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ForegroundSelectionActor = vtkSmartPointer<vtkActor>::New();
  this->ForegroundSelectionActor->SetMapper(this->ForegroundSelectionMapper);
  this->ForegroundSelectionActor->GetProperty()->SetLineWidth(4);
  this->ForegroundSelectionActor->GetProperty()->SetColor(0,1,0);
  this->ForegroundSelectionMapper->SetInputConnection(this->ForegroundSelection->GetProducerPort());

  // Background
  this->BackgroundSelection = vtkSmartPointer<vtkPolyData>::New();
  this->BackgroundSelectionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->BackgroundSelectionActor = vtkSmartPointer<vtkActor>::New();
  this->BackgroundSelectionActor->SetMapper(this->BackgroundSelectionMapper);
  this->BackgroundSelectionActor->GetProperty()->SetLineWidth(4);
  this->BackgroundSelectionActor->GetProperty()->SetColor(1,0,0);
  this->BackgroundSelectionMapper->SetInputConnection(this->BackgroundSelection->GetProducerPort());

  //this->ResultActor = vtkSmartPointer<vtkImageActor>::New();

  this->Tracer->AddObserver(vtkCommand::EndInteractionEvent, this, &vtkGraphCutInteractorStyle::CatchWidgetEvent);

  // Defaults
  this->SelectionType = FOREGROUND;
}

vtkPolyData* vtkGraphCutInteractorStyle::GetForegroundSelection()
{
  return this->ForegroundSelection;
}

vtkPolyData* vtkGraphCutInteractorStyle::GetBackgroundSelection()
{
  return this->BackgroundSelection;
}

int vtkGraphCutInteractorStyle::GetSelectionType()
{
  return this->SelectionType;
}

void vtkGraphCutInteractorStyle::InitializeTracer(vtkImageActor* imageActor)
{
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(imageActor);
  this->Tracer->SetInteractor(this->Interactor);
  this->Tracer->SetViewProp(imageActor);
  this->Tracer->ProjectToPlaneOn();

  this->Tracer->On();
}

void vtkGraphCutInteractorStyle::SetInteractionModeToForeground()
{
  this->Tracer->GetLineProperty()->SetColor(0,1,0);
  this->SelectionType = FOREGROUND;
}

void vtkGraphCutInteractorStyle::SetInteractionModeToBackground()
{
  this->Tracer->GetLineProperty()->SetColor(1,0,0);
  this->SelectionType = BACKGROUND;
}

void vtkGraphCutInteractorStyle::CatchWidgetEvent(vtkObject* caller, long unsigned int eventId, void* callData)
{
  // Get the path from the tracer and append it to the appropriate selection

  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(BackgroundSelectionActor);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(ForegroundSelectionActor);

  vtkImageTracerWidget* tracer =
    static_cast<vtkImageTracerWidget*>(caller);

  vtkSmartPointer<vtkPolyData> path =
    vtkSmartPointer<vtkPolyData>::New();
  tracer->GetPath(path);

  vtkSmartPointer<vtkAppendPolyData> appendFilter =
    vtkSmartPointer<vtkAppendPolyData>::New();
  appendFilter->AddInputConnection(path->GetProducerPort());

  if(this->SelectionType == vtkGraphCutInteractorStyle::FOREGROUND)
    {
    appendFilter->AddInputConnection(this->ForegroundSelection->GetProducerPort());
    appendFilter->Update();
    this->ForegroundSelection->ShallowCopy(appendFilter->GetOutput());
    }
  else if(this->SelectionType == vtkGraphCutInteractorStyle::BACKGROUND)
    {
    appendFilter->AddInputConnection(this->BackgroundSelection->GetProducerPort());
    appendFilter->Update();
    this->BackgroundSelection->ShallowCopy(appendFilter->GetOutput());
    }

  vtkSmartPointer<vtkPoints> emptyPoints =
    vtkSmartPointer<vtkPoints>::New();
  emptyPoints->InsertNextPoint(0, 0, 0);
  emptyPoints->InsertNextPoint(0, 0, 0);

  this->Tracer->InitializeHandles(emptyPoints);
  this->Tracer->Modified();

  this->Refresh();

};

void vtkGraphCutInteractorStyle::Refresh()
{
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();
  this->Interactor->GetRenderWindow()->Render();
}

void vtkGraphCutInteractorStyle::ClearSelections()
{
  /*
   // I thought this would work...
  this->BackgroundSelection->Reset();
  this->BackgroundSelection->Squeeze();
  this->BackgroundSelection->Modified();

  this->ForegroundSelection->Reset();
  this->ForegroundSelection->Squeeze();
  this->ForegroundSelection->Modified();
  */

  // This seems like a silly way of emptying the polydatas...
  vtkSmartPointer<vtkPolyData> empytPolyData =
    vtkSmartPointer<vtkPolyData>::New();
  this->ForegroundSelection->ShallowCopy(empytPolyData);
  this->BackgroundSelection->ShallowCopy(empytPolyData);

  this->Refresh();

}
