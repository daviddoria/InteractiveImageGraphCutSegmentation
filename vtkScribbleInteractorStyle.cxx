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

#include "vtkScribbleInteractorStyle.h"

// Custom
#include "Helpers.h"

// VTK
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
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
#include <vtkVertexGlyphFilter.h>

// ITK
#include <itkImageRegionIterator.h>
#include <itkBresenhamLine.h>

vtkStandardNewMacro(vtkScribbleInteractorStyle);

vtkScribbleInteractorStyle::vtkScribbleInteractorStyle()
{
  // Initializations
  this->Tracer = vtkSmartPointer<vtkImageTracerWidget>::New();
  this->Tracer->GetLineProperty()->SetLineWidth(5);
  this->Tracer->HandleMiddleMouseButtonOff();

  // Foreground
  this->ForegroundSelectionPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->ForegroundSelectionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ForegroundSelectionActor = vtkSmartPointer<vtkActor>::New();
  this->ForegroundSelectionActor->SetMapper(this->ForegroundSelectionMapper);
  this->ForegroundSelectionActor->GetProperty()->SetLineWidth(4);
  this->ForegroundSelectionActor->GetProperty()->SetColor(0,1,0);
  this->ForegroundSelectionMapper->SetInputData(this->ForegroundSelectionPolyData);

  // Background
  this->BackgroundSelectionPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->BackgroundSelectionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->BackgroundSelectionActor = vtkSmartPointer<vtkActor>::New();
  this->BackgroundSelectionActor->SetMapper(this->BackgroundSelectionMapper);
  this->BackgroundSelectionActor->GetProperty()->SetLineWidth(4);
  this->BackgroundSelectionActor->GetProperty()->SetColor(1,0,0);
  this->BackgroundSelectionMapper->SetInputData(this->BackgroundSelectionPolyData);

  // Update the selection when the EndInteraction event is fired.
  this->Tracer->AddObserver(vtkCommand::EndInteractionEvent, this, &vtkScribbleInteractorStyle::CatchWidgetEvent);

  // Defaults
  this->SelectionType = FOREGROUND;
}

void vtkScribbleInteractorStyle::SetForegroundSelection(const std::vector<itk::Index<2> >& pixels)
{
  this->ForegroundSelection = pixels;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  Helpers::PixelListToPoints(pixels, points);

  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  
  vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
    vtkSmartPointer<vtkVertexGlyphFilter>::New();
  vertexGlyphFilter->AddInputData(polydata);
  vertexGlyphFilter->Update();
  
  ForegroundSelectionPolyData->DeepCopy(vertexGlyphFilter->GetOutput());
  ForegroundSelectionMapper->Modified();
  this->Refresh();
}

void vtkScribbleInteractorStyle::SetBackgroundSelection(const std::vector<itk::Index<2> >& pixels)
{
  this->BackgroundSelection = pixels;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  Helpers::PixelListToPoints(pixels, points);

  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
    vtkSmartPointer<vtkVertexGlyphFilter>::New();
  vertexGlyphFilter->AddInputData(polydata);
  vertexGlyphFilter->Update();

  BackgroundSelectionPolyData->DeepCopy(vertexGlyphFilter->GetOutput());
  BackgroundSelectionMapper->Modified();
  this->Refresh();
}

std::vector<itk::Index<2> > vtkScribbleInteractorStyle::GetForegroundSelection()
{
  return this->ForegroundSelection;
}

std::vector<itk::Index<2> > vtkScribbleInteractorStyle::GetBackgroundSelection()
{
  return this->BackgroundSelection;
}

int vtkScribbleInteractorStyle::GetSelectionType()
{
  return this->SelectionType;
}

void vtkScribbleInteractorStyle::InitializeTracer(vtkImageActor* imageActor)
{
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(imageActor);
  this->Tracer->SetInteractor(this->Interactor);
  this->Tracer->SetViewProp(imageActor);
  this->Tracer->ProjectToPlaneOn();

  this->Tracer->On();
}

void vtkScribbleInteractorStyle::SetInteractionModeToForeground()
{
  this->Tracer->GetLineProperty()->SetColor(0,1,0);
  this->SelectionType = FOREGROUND;
}

void vtkScribbleInteractorStyle::SetInteractionModeToBackground()
{
  this->Tracer->GetLineProperty()->SetColor(1,0,0);
  this->SelectionType = BACKGROUND;
}

void vtkScribbleInteractorStyle::Init()
{
  std::cout << "Initialized renderer " << this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer() << std::endl;
  
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(BackgroundSelectionActor);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(ForegroundSelectionActor);
}

void vtkScribbleInteractorStyle::CatchWidgetEvent(vtkObject* caller, long unsigned int eventId, void* callData)
{
  // Get the path from the tracer and append it to the appropriate selection
  std::cout << "Using renderer " << this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer() << std::endl;

  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(BackgroundSelectionActor);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(ForegroundSelectionActor);

  // Get the tracer object (this is the object that triggered this event)
  vtkImageTracerWidget* tracer =
    static_cast<vtkImageTracerWidget*>(caller);

  // Get the points in the selection
  vtkSmartPointer<vtkPolyData> path =
    vtkSmartPointer<vtkPolyData>::New();
  tracer->GetPath(path);

  // Create a filter which will be used to combine the most recent selection with previous selections
  vtkSmartPointer<vtkAppendPolyData> appendFilter =
    vtkSmartPointer<vtkAppendPolyData>::New();
  appendFilter->AddInputData(path);

  std::vector<itk::Index<2> > newPoints = Helpers::PolyDataToPixelList(path);
  //std::cout << newPoints.size() << " new points." << std::endl;

  // If we are in foreground mode, add the current selection to the foreground. Else, add it to the background.
  if(this->SelectionType == vtkScribbleInteractorStyle::FOREGROUND)
    {
    appendFilter->AddInputData(this->ForegroundSelectionPolyData);
    appendFilter->Update();
    this->ForegroundSelectionPolyData->ShallowCopy(appendFilter->GetOutput());

    this->ForegroundSelection.insert(this->ForegroundSelection.end(), newPoints.begin(), newPoints.end());
    }
  else if(this->SelectionType == vtkScribbleInteractorStyle::BACKGROUND)
    {
    appendFilter->AddInputData(this->BackgroundSelectionPolyData);
    appendFilter->Update();
    this->BackgroundSelectionPolyData->ShallowCopy(appendFilter->GetOutput());

    this->BackgroundSelection.insert(this->BackgroundSelection.end(), newPoints.begin(), newPoints.end());
    }

  //std::cout << this->ForegroundSelection.size() << " foreground poitns." << std::endl;
  //std::cout << this->BackgroundSelection.size() << " background poitns." << std::endl;

  // "Clear" the tracer. We must rely on the foreground and background actors to maintain the appropriate colors.
  // If we did not clear the tracer, if we draw a foreground stroke (green) then switch to background mode, the last stoke would turn
  // red until we finished drawing the next stroke.
  vtkSmartPointer<vtkPoints> emptyPoints =
    vtkSmartPointer<vtkPoints>::New();
  emptyPoints->InsertNextPoint(0, 0, 0);
  emptyPoints->InsertNextPoint(0, 0, 0);

  this->Tracer->InitializeHandles(emptyPoints);
  this->Tracer->Modified();

  this->Refresh();

};

void vtkScribbleInteractorStyle::Refresh()
{
  //this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();
  this->Interactor->GetRenderWindow()->Render();
}

void vtkScribbleInteractorStyle::ClearSelections()
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
  this->ForegroundSelectionPolyData->ShallowCopy(empytPolyData);
  this->BackgroundSelectionPolyData->ShallowCopy(empytPolyData);

  this->ForegroundSelection.clear();
  this->BackgroundSelection.clear();

  this->Refresh();

}

void vtkScribbleInteractorStyle::ClearForegroundSelections()
{
  // This seems like a silly way of emptying the polydatas...
  vtkSmartPointer<vtkPolyData> empytPolyData =
    vtkSmartPointer<vtkPolyData>::New();
  this->ForegroundSelectionPolyData->ShallowCopy(empytPolyData);
  this->ForegroundSelection.clear();
  this->Refresh();
}

void vtkScribbleInteractorStyle::ClearBackgroundSelections()
{
  // This seems like a silly way of emptying the polydatas...
  vtkSmartPointer<vtkPolyData> empytPolyData =
    vtkSmartPointer<vtkPolyData>::New();
  this->BackgroundSelectionPolyData->ShallowCopy(empytPolyData);

  this->BackgroundSelection.clear();

  this->Refresh();
}
