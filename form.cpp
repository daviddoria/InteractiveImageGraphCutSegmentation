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

// Qt
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>

Form::Form(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
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

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();

  // Instantiations
  this->OriginalImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();

  // Add renderers - we flip the image by changing the camera view up because of the conflicting conventions used by ITK and VTK
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

  // Default GUI settings
  this->radForeground->setChecked(true);

  // Instantiations
  this->GraphCut = NULL;
}


void Form::StartProgressSlot()
{
  // Connected to the StartProgressSignal of the ProgressThread member
  this->progressBar->show();
}

void Form::StopProgressSlot()
{
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  // Convert the segment mask image into a VTK image for display
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  ITKImagetoVTKImage<GrayscaleImageType>(this->GraphCut->GetSegmentMask(), VTKSegmentMask);

  // Remove the old output, set the new output and refresh everything
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
  // Compute lambda by multiplying the percentage set by the slider by the MaxLambda set in the text box

  double lambdaMax = this->txtLambdaMax->text().toDouble();
  double lambdaPercent = this->sldLambda->value()/100.;
  double lambda = lambdaPercent * lambdaMax;

  return lambda;
}

void Form::UpdateLambda()
{
  // Compute lambda and then set the label to this value so the user can see the current setting
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
  // Ask the user for a filename to save the segment mask image to
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Open Image"), "/home/doriad", tr("Image Files (*.png *.jpg *.bmp)"));

  // Write the file
  typedef  itk::ImageFileWriter< GrayscaleImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  writer->SetInput(this->GraphCut->GetSegmentMask());
  writer->Update();
}

void Form::btnCut_clicked()
{
  // Get the number of bins from the slider
  this->GraphCut->SetNumberOfHistogramBins(this->sldHistogramBins->value());

  if(this->sldLambda->value() == 0)
    {
    QMessageBox msgBox;
    msgBox.setText("You must select lambda > 0!");
    msgBox.exec();
    return;
    }

  // Setup the graph cut from the GUI and the scribble selection
  this->GraphCut->SetLambda(ComputeLambda());
  this->GraphCut->SetSources(this->GraphCutStyle->GetForegroundSelection());
  this->GraphCut->SetSinks(this->GraphCutStyle->GetBackgroundSelection());

  // Setup and start the actual cut computation in a different thread
  this->ProgressThread.GraphCut = this->GraphCut;
  ProgressThread.start();

}

void Form::actionOpen_Grayscale_Image_triggered()
{
  // Instantiate everything to use grayscale images and grayscale histograms
  OpenFile<GrayscaleImageType>();
}

void Form::actionOpen_Color_Image_triggered()
{
  // Instantiate everything to use color images and color histograms
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
    return;
    }

  // Clear the scribbles
  this->GraphCutStyle->ClearSelections();

  // Read file
  typedef itk::ImageFileReader<TImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.toStdString());
  reader->Update();

  // Delete the old object if one exists
  if(this->GraphCut)
    {
    delete this->GraphCut;
    }

  // Instantiate the ImageGraphCut object with the correct type
  this->GraphCut = new ImageGraphCut<TImageType>(reader->GetOutput());

  // Convert the ITK image to a VTK image and display it
  vtkSmartPointer<vtkImageData> VTKImage =
    vtkSmartPointer<vtkImageData>::New();
  ITKImagetoVTKImage<TImageType>(reader->GetOutput(), VTKImage);

  this->OriginalImageActor->SetInput(VTKImage);
  this->GraphCutStyle->InitializeTracer(this->OriginalImageActor);

  this->LeftRenderer->ResetCamera();
  this->Refresh();

  // Setup the scribble style
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
  // Setup and allocate the image data
  outputImage->SetNumberOfScalarComponents(TImageType::PixelType::GetNumberOfComponents());
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the input image pixels to the output image
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

// Explit instantiations
template void ITKImagetoVTKImage<ColorImageType>(ColorImageType::Pointer, vtkImageData*);
template void ITKImagetoVTKImage<GrayscaleImageType>(GrayscaleImageType::Pointer, vtkImageData*);
