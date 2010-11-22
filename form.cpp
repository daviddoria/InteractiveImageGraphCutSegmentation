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

#include "form.h"

// ITK
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// VTK
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <QFileDialog>
#include <QLineEdit>

Form::Form(QWidget *parent)
{
  setupUi(this);
  connect( this->actionOpen_Color_Image, SIGNAL( triggered() ), this, SLOT(actionOpen_Color_Image_triggered()) );
  connect( this->actionOpen_Grayscale_Image, SIGNAL( triggered() ), this, SLOT(actionOpen_Grayscale_Image_triggered()) );
  connect( this->radForeground, SIGNAL( clicked() ), this, SLOT(radForeground_clicked()) );
  connect( this->radBackground, SIGNAL( clicked() ), this, SLOT(radBackground_clicked()) );
  connect( this->btnCut, SIGNAL( clicked() ), this, SLOT(btnCut_clicked()));
  connect( this->btnSave, SIGNAL( clicked() ), this, SLOT(btnSave_clicked()));
  connect( this->btnClearSelections, SIGNAL( clicked() ), this, SLOT(btnClearSelections_clicked()));
  connect( this->sldHistogramBins, SIGNAL( valueChanged(int) ), this, SLOT(sldHistogramBins_valueChanged()));
  connect( this->sldLambda, SIGNAL( valueChanged(int) ), this, SLOT(UpdateLambda()));
  connect( this->txtLambdaMax, SIGNAL( textEdited(QString) ), this, SLOT(UpdateLambda()));
  connect(&ProgressThread, SIGNAL(StartProgressSignal()), this, SLOT(StartProgressSlot()), Qt::QueuedConnection);
  connect(&ProgressThread, SIGNAL(StopProgressSignal()), this, SLOT(StopProgressSlot()), Qt::QueuedConnection);

  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();

  // Instantiations
  this->OriginalImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();

  // Add renderers
  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->LeftRenderer->GetActiveCamera()->SetViewUp(0,-1,0);
  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);

  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer->GetActiveCamera()->SetViewUp(0,-1,0);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  // Setup right interactor style
  vtkSmartPointer<vtkInteractorStyleImage> interactorStyleImage =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetInteractor()->SetInteractorStyle(interactorStyleImage);

  // Setup left interactor style
  this->GraphCutStyle = vtkSmartPointer<vtkGraphCutInteractorStyle>::New();
  this->qvtkWidgetLeft->GetInteractor()->SetInteractorStyle(this->GraphCutStyle);

  this->radForeground->setChecked(true);

  this->GraphCut = NULL;
}


void Form::StartProgressSlot()
{
  this->progressBar->show();
}

void Form::StopProgressSlot()
{
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  ITKImagetoVTKImage<GrayscaleImageType>(this->GraphCut->GetSegmentMask(), VTKSegmentMask);
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor->SetInput(VTKSegmentMask);
  this->RightRenderer->RemoveAllViewProps();
  this->RightRenderer->AddActor(ResultActor);
  this->RightRenderer->ResetCamera();
  this->Refresh();

  this->progressBar->hide();
}

float Form::ComputeLambda()
{
  double lambdaMax = this->txtLambdaMax->text().toDouble();
  double lambdaPercent = this->sldLambda->value()/100.;
  double lambda = lambdaPercent * lambdaMax;
  return lambda;
}

void Form::UpdateLambda()
{
  double lambda = ComputeLambda();
  this->lblLambda->setText(QString::number(lambda));
}

void Form::sldHistogramBins_valueChanged()
{
  this->GraphCut->SetNumberOfHistogramBins(sldHistogramBins->value());
}

void Form::radForeground_clicked()
{
  this->GraphCutStyle->SetInteractionModeToForeground();
}

void Form::radBackground_clicked()
{
  this->GraphCutStyle->SetInteractionModeToBackground();
}

void Form::btnClearSelections_clicked()
{
  this->GraphCutStyle->ClearSelections();
}

void Form::btnSave_clicked()
{
  // Set a filename to save
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Open Image"), "/home/doriad", tr("Image Files (*.png *.jpg *.bmp)"));

  typedef  itk::ImageFileWriter< GrayscaleImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  writer->SetInput(this->GraphCut->GetSegmentMask());
  writer->Update();
}

void Form::btnCut_clicked()
{
  this->GraphCut->SetLambda(ComputeLambda());
  this->GraphCut->SetSources(this->GraphCutStyle->GetForegroundSelection());
  this->GraphCut->SetSinks(this->GraphCutStyle->GetBackgroundSelection());
  //this->GraphCut->PerformSegmentation();

  this->ProgressThread.GraphCut = this->GraphCut;
  ProgressThread.start();
  //ProgressThread.wait();

  /*
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  ITKImagetoVTKImage<GrayscaleImageType>(this->GraphCut->GetSegmentMask(), VTKSegmentMask);
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor->SetInput(VTKSegmentMask);
  this->RightRenderer->RemoveAllViewProps();
  this->RightRenderer->AddActor(ResultActor);
  this->RightRenderer->ResetCamera();
  this->Refresh();
  */
}

void Form::actionOpen_Grayscale_Image_triggered()
{
  OpenFile<GrayscaleImageType>();
}

void Form::actionOpen_Color_Image_triggered()
{
  OpenFile<ColorImageType>();
}

template <typename TImageType>
void Form::OpenFile()
{
  // Get a filename to open
  QString filename = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/media/portable/Projects/src/GrabCut/data", tr("Image Files (*.png *.jpg *.bmp)"));

  if(filename.isEmpty())
    {
    std::cout << "Cancelled." << std::endl;
    return;
    }

  //std::cout << "Got filename: " << filename.toStdString() << std::endl;

  this->GraphCutStyle->ClearSelections();

  // Read file
  typedef itk::ImageFileReader<TImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.toStdString());
  reader->Update();

  if(this->GraphCut)
    {
    delete this->GraphCut;
    }
  this->GraphCut = new ImageGraphCut<TImageType>(reader->GetOutput());

  vtkSmartPointer<vtkImageData> VTKImage =
    vtkSmartPointer<vtkImageData>::New();
  ITKImagetoVTKImage<TImageType>(reader->GetOutput(), VTKImage);

  this->OriginalImageActor->SetInput(VTKImage);
  this->GraphCutStyle->InitializeTracer(this->OriginalImageActor);

  this->LeftRenderer->ResetCamera();
  this->Refresh();

  if(this->radBackground->isChecked())
    {
    this->GraphCutStyle->SetInteractionModeToBackground();
    }
  else
    {
    this->GraphCutStyle->SetInteractionModeToForeground();
    }
}

void Form::Refresh()
{
  this->LeftRenderer->Render();
  this->RightRenderer->Render();
  this->qvtkWidgetRight->GetRenderWindow()->Render();
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
  this->qvtkWidgetRight->GetInteractor()->Render();
  this->qvtkWidgetLeft->GetInteractor()->Render();
}

template <typename TImageType>
void ITKImagetoVTKImage(typename TImageType::Pointer image, vtkImageData* outputImage)
{
  // Specify the size of the image data
  outputImage->SetNumberOfScalarComponents(TImageType::PixelType::GetNumberOfComponents());
  outputImage->SetScalarTypeToUnsignedChar();

  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  itk::ImageRegionConstIteratorWithIndex<TImageType> imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    for(unsigned int component = 0; component < imageIterator.Get().Size(); component++)
      {
      pixel[component] = static_cast<unsigned char>(imageIterator.Get()[component]);
      }

    ++imageIterator;
    }
}

template void ITKImagetoVTKImage<ColorImageType>(ColorImageType::Pointer, vtkImageData*);
template void ITKImagetoVTKImage<GrayscaleImageType>(GrayscaleImageType::Pointer, vtkImageData*);
