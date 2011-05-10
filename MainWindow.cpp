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

#include "MainWindow.h"

#include "Helpers.h"

// ITK
#include <itkCastImageFilter.h>
#include <itkCovariantVector.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkLineIterator.h>
#include <itkNthElementImageAdaptor.h>

// VTK
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

// Qt
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);
  
  // Menu items
  // File menu
  connect( this->actionOpenImage, SIGNAL( triggered() ), this, SLOT(actionOpenImage_triggered()) );
  connect( this->actionSaveSegmentation, SIGNAL( triggered() ), this, SLOT(actionSaveSegmentation_triggered()));

  // Edit menu
  connect( this->actionFlipImage, SIGNAL( triggered() ), this, SLOT(actionFlipImage_triggered()));
  
  connect( this->radForeground, SIGNAL( clicked() ), this, SLOT(radForeground_clicked()) );
  connect( this->radBackground, SIGNAL( clicked() ), this, SLOT(radBackground_clicked()) );
  connect( this->btnCut, SIGNAL( clicked() ), this, SLOT(btnCut_clicked()));
  connect( this->btnClearSelections, SIGNAL( clicked() ), this, SLOT(btnClearSelections_clicked()));
  connect( this->btnSaveSelections, SIGNAL( clicked() ), this, SLOT(btnSaveSelections_clicked()));
  connect( this->sldHistogramBins, SIGNAL( valueChanged(int) ), this, SLOT(sldHistogramBins_valueChanged()));
  connect( this->sldLambda, SIGNAL( valueChanged(int) ), this, SLOT(UpdateLambda()));
  connect( this->txtLambdaMax, SIGNAL( textEdited(QString) ), this, SLOT(UpdateLambda()));
  connect(&SegmentationThread, SIGNAL(StartProgressSignal()), this, SLOT(StartProgressSlot()), Qt::QueuedConnection);
  connect(&SegmentationThread, SIGNAL(StopProgressSignal()), this, SLOT(StopProgressSlot()), Qt::QueuedConnection);

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();

  this->BackgroundColor[0] = 0;
  this->BackgroundColor[1] = 0;
  this->BackgroundColor[2] = .5;

  this->CameraUp[0] = 0;
  this->CameraUp[1] = 1;
  this->CameraUp[2] = 0;

  // Instantiations
  this->OriginalImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();

  // Add renderers - we flip the image by changing the camera view up because of the conflicting conventions used by ITK and VTK
  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->LeftRenderer->GradientBackgroundOn();
  this->LeftRenderer->SetBackground(this->BackgroundColor);
  this->LeftRenderer->SetBackground2(1,1,1);
  this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);

  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer->GradientBackgroundOn();
  this->RightRenderer->SetBackground(this->BackgroundColor);
  this->RightRenderer->SetBackground2(1,1,1);
  this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  // Setup right interactor style
  vtkSmartPointer<vtkInteractorStyleImage> interactorStyleImage =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetInteractor()->SetInteractorStyle(interactorStyleImage);

  // Setup left interactor style
  this->GraphCutStyle = vtkSmartPointer<vtkScribbleInteractorStyle>::New();
  this->qvtkWidgetLeft->GetInteractor()->SetInteractorStyle(this->GraphCutStyle);

  // Default GUI settings
  this->radForeground->setChecked(true);
}

void MainWindow::actionFlipImage_triggered()
{
  this->CameraUp[1] *= -1;
  this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->Refresh();
}

void MainWindow::actionSaveSegmentation_triggered()
{
  // Ask the user for a filename to save the segment mask image to

  QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Segment Mask Image"), "/home/doriad", tr("Image Files (*.png *.bmp)"));
/*
  // Convert the image from a 1D vector image to an unsigned char image
  typedef itk::CastImageFilter< GrayscaleImageType, itk::Image<itk::CovariantVector<unsigned char, 1>, 2 > > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(this->GraphCut->GetSegmentMask());

  typedef itk::NthElementImageAdaptor< itk::Image<itk:: CovariantVector<unsigned char, 1>, 2 >,
    unsigned char> ImageAdaptorType;

  ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
  adaptor->SelectNthElement(0);
  adaptor->SetImage(castFilter->GetOutput());
*/
  // Write the file
  //typedef  itk::ImageFileWriter< ImageAdaptorType > WriterType;
  typedef  itk::ImageFileWriter< MaskImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  //writer->SetInput(adaptor);
  writer->SetInput(this->GraphCut.GetSegmentMask());
  writer->Update();

}

void MainWindow::actionOpenImage_triggered()
{
  std::cout << "actionOpenImage_triggered()" << std::endl;
  OpenFile();
}


void MainWindow::StartProgressSlot()
{
  // Connected to the StartProgressSignal of the ProgressThread member
  this->progressBar->show();
}

/*
 // Display segmented image with black background pixels
void Form::StopProgressSlot()
{
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  // Convert the masked image into a VTK image for display
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  if(this->GraphCut->GetPixelDimensionality() == 1)
    {
    ITKImagetoVTKImage<GrayscaleImageType>(static_cast<ImageGraphCut<GrayscaleImageType>* >(this->GraphCut)->GetMaskedOutput(), VTKSegmentMask);
    }
  else if(this->GraphCut->GetPixelDimensionality() == 3)
    {
    ITKImagetoVTKImage<ColorImageType>(static_cast<ImageGraphCut<ColorImageType>* >(this->GraphCut)->GetMaskedOutput(), VTKSegmentMask);
    }
  else if(this->GraphCut->GetPixelDimensionality() == 5)
    {
    ITKImagetoVTKImage<RGBDIImageType>(static_cast<ImageGraphCut<RGBDIImageType>* >(this->GraphCut)->GetMaskedOutput(), VTKSegmentMask);
    }
  else
    {
    std::cerr << "This type of image (" << this->GraphCut->GetPixelDimensionality() << ") cannot be displayed!" << std::endl;
    exit(-1);
    }

  // Remove the old output, set the new output and refresh everything
  this->ResultActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor->SetInput(VTKSegmentMask);
  this->RightRenderer->RemoveAllViewProps();
  this->RightRenderer->AddActor(ResultActor);
  this->RightRenderer->ResetCamera();
  this->Refresh();

  this->progressBar->hide();
}
*/

// Display segmented image with transparent background pixels
void MainWindow::StopProgressSlot()
{
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  // Convert the segmentation mask to a binary VTK image
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKImagetoVTKImage(this->GraphCut.GetSegmentMask(), VTKSegmentMask);

  // Convert the image into a VTK image for display
  vtkSmartPointer<vtkImageData> VTKImage =
    vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKImagetoVTKImage(this->GraphCut.GetMaskedOutput(), VTKImage);

  vtkSmartPointer<vtkImageData> VTKMaskedImage =
    vtkSmartPointer<vtkImageData>::New();
  Helpers::MaskImage(VTKImage, VTKSegmentMask, VTKMaskedImage);

  // Remove the old output, set the new output and refresh everything
  //this->ResultActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultActor->SetInput(VTKMaskedImage);
  this->RightRenderer->RemoveAllViewProps();
  this->RightRenderer->AddActor(ResultActor);
  this->RightRenderer->ResetCamera();
  this->Refresh();

  this->progressBar->hide();
}

float MainWindow::ComputeLambda()
{
  // Compute lambda by multiplying the percentage set by the slider by the MaxLambda set in the text box

  double lambdaMax = this->txtLambdaMax->text().toDouble();
  double lambdaPercent = this->sldLambda->value()/100.;
  double lambda = lambdaPercent * lambdaMax;

  return lambda;
}

void MainWindow::UpdateLambda()
{
  // Compute lambda and then set the label to this value so the user can see the current setting
  double lambda = ComputeLambda();
  this->lblLambda->setText(QString::number(lambda));
}

void MainWindow::sldHistogramBins_valueChanged()
{
  this->GraphCut.SetNumberOfHistogramBins(sldHistogramBins->value());
}

void MainWindow::radForeground_clicked()
{
  this->GraphCutStyle->SetInteractionModeToForeground();
}

void MainWindow::radBackground_clicked()
{
  this->GraphCutStyle->SetInteractionModeToBackground();
}

void MainWindow::btnClearSelections_clicked()
{
  this->GraphCutStyle->ClearSelections();
}

void MainWindow::btnSaveSelections_clicked()
{
  QString directoryName = QFileDialog::getExistingDirectory(this,
     "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly);

  std::string foregroundFilename = QDir(directoryName).absoluteFilePath("foreground.png").toStdString();
  std::string backgroundFilename = QDir(directoryName).absoluteFilePath("background.png").toStdString();

  std::cout << "Writing to " << foregroundFilename << " and " << backgroundFilename << std::endl;

  UnsignedCharScalarImageType::Pointer foregroundImage = UnsignedCharScalarImageType::New();
  foregroundImage->SetRegions(this->ImageRegion);
  foregroundImage->Allocate();
  Helpers::IndicesToBinaryImage(this->GraphCutStyle->GetForegroundSelection(), foregroundImage);

  typedef  itk::ImageFileWriter< UnsignedCharScalarImageType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(foregroundFilename);
  writer->SetInput(foregroundImage);
  writer->Update();

  UnsignedCharScalarImageType::Pointer backgroundImage = UnsignedCharScalarImageType::New();
  backgroundImage->SetRegions(this->ImageRegion);
  backgroundImage->Allocate();
  Helpers::IndicesToBinaryImage(this->GraphCutStyle->GetBackgroundSelection(), backgroundImage);

  writer->SetFileName(backgroundFilename);
  writer->SetInput(backgroundImage);
  writer->Update();
}

void MainWindow::btnCut_clicked()
{
  // Get the number of bins from the slider
  this->GraphCut.SetNumberOfHistogramBins(this->sldHistogramBins->value());

  if(this->sldLambda->value() == 0)
    {
    QMessageBox msgBox;
    msgBox.setText("You must select lambda > 0!");
    msgBox.exec();
    return;
    }

  // Setup the graph cut from the GUI and the scribble selection
  this->GraphCut.SetLambda(ComputeLambda());
  this->GraphCut.SetSources(this->GraphCutStyle->GetForegroundSelection());
  this->GraphCut.SetSinks(this->GraphCutStyle->GetBackgroundSelection());

  // Setup and start the actual cut computation in a different thread
  this->SegmentationThread.GraphCut = &(this->GraphCut);
  SegmentationThread.start();

}

#if 0
void InnerWidget::actionFlip_Image_triggered()
{
  this->CameraUp[1] *= -1;
  this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->Refresh();
}
#endif

#if 0
void InnerWidget::actionSave_Segmentation_triggered()
{
  // Ask the user for a filename to save the segment mask image to

  QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Segment Mask Image"), "/home/doriad", tr("Image Files (*.png *.bmp)"));
/*
  // Convert the image from a 1D vector image to an unsigned char image
  typedef itk::CastImageFilter< GrayscaleImageType, itk::Image<itk::CovariantVector<unsigned char, 1>, 2 > > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(this->GraphCut->GetSegmentMask());

  typedef itk::NthElementImageAdaptor< itk::Image<itk:: CovariantVector<unsigned char, 1>, 2 >,
    unsigned char> ImageAdaptorType;

  ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
  adaptor->SelectNthElement(0);
  adaptor->SetImage(castFilter->GetOutput());
*/
  // Write the file
  //typedef  itk::ImageFileWriter< ImageAdaptorType > WriterType;
  typedef  itk::ImageFileWriter< MaskImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  //writer->SetInput(adaptor);
  writer->SetInput(this->GraphCut->GetSegmentMask());
  writer->Update();

}
#endif

void MainWindow::OpenFile()
{
  std::cout << "OpenFile()" << std::endl;
  
  // Get a filename to open
  QString filename = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/media/portable/Projects/src/InteractiveImageGraphCutSegmentation/data", tr("Image Files (*.png *.mhd)"));

  if(filename.isEmpty())
    {
    return;
    }

  // Clear the scribbles
  this->GraphCutStyle->ClearSelections();

  // Read file
  typename itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();

  reader->SetFileName(filename.toStdString());
  reader->Update();

  this->ImageRegion = reader->GetOutput()->GetLargestPossibleRegion();

  this->GraphCut.SetImage(reader->GetOutput());

  // Convert the ITK image to a VTK image and display it
  vtkSmartPointer<vtkImageData> VTKImage =
    vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKImagetoVTKImage(reader->GetOutput(), VTKImage);

  this->LeftRenderer->RemoveAllViewProps();

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

void MainWindow::Refresh()
{
  this->LeftRenderer->Render();
  this->RightRenderer->Render();
  this->qvtkWidgetRight->GetRenderWindow()->Render();
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
  this->qvtkWidgetRight->GetInteractor()->Render();
  this->qvtkWidgetLeft->GetInteractor()->Render();
}
