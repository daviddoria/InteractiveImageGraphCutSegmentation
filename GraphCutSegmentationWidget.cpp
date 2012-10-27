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

#include "GraphCutSegmentationWidget.h"

// Submodules
#include "Mask/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "Mask/ITKHelpers/ITKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"
#include "Mask/Mask.h"

// ITK
#include <itkCastImageFilter.h>
#include <itkCovariantVector.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkLineIterator.h>
#include <itkNthElementImageAdaptor.h>

// VTK
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPNGWriter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

// Qt
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QtConcurrentRun>

// STL
#include <iostream>

GraphCutSegmentationWidget::GraphCutSegmentationWidget(const std::string& fileName) : QMainWindow(NULL)
{
  SharedConstructor();
  OpenFile(fileName);
}

GraphCutSegmentationWidget::GraphCutSegmentationWidget(QWidget *parent) : QMainWindow(parent)
{
  SharedConstructor();
}

void GraphCutSegmentationWidget::SharedConstructor()
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);

  this->BackgroundColor[0] = 0;
  this->BackgroundColor[1] = 0;
  this->BackgroundColor[2] = 1;
  
  SelectedPixelSet = &Sources;
  
  SourceSinkImageData = vtkSmartPointer<vtkImageData>::New();
  SetupBothPanes(); // This must be called before SetupLeftPane() and SetupRightPane()
  SetupLeftPane();
  SetupRightPane();

  // This will track if we should reset the camera or not
  this->AlreadySegmented = false;

  // Setup the progress bar
  this->ProgressDialog = new QProgressDialog();
  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  connect(&this->FutureWatcher, SIGNAL(finished()), this, SLOT(slot_SegmentationComplete()));
  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));
  
  connect( this->sldHistogramBins, SIGNAL( valueChanged(int) ), this, SLOT(sldHistogramBins_valueChanged()));
  connect( this->sldLambda, SIGNAL( valueChanged(int) ), this, SLOT(UpdateLambda()));
  connect( this->txtLambdaMax, SIGNAL( textEdited(QString) ), this, SLOT(UpdateLambda()));

  SetupCameras();

  // Default GUI settings
  this->radForeground->setChecked(true);

  // Setup toolbar
  // Open file buttons
  QIcon openIcon = QIcon::fromTheme("document-open");
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  // Save buttons
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSaveSegmentation->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveSegmentation);
}

void GraphCutSegmentationWidget::SetupBothPanes()
{
  this->OriginalImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->OriginalImageSlice->VisibilityOff();
  this->OriginalImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->OriginalImageSlice->SetMapper(this->OriginalImageSliceMapper);
  
  this->ResultSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ResultSlice ->VisibilityOff();
  this->ResultSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ResultSlice->SetMapper(this->ResultSliceMapper);
}

void GraphCutSegmentationWidget::SetupLeftPane()
{
  // Add renderers - we flip the image by changing the camera view up because of the conflicting
  // conventions used by ITK and VTK
  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->LeftRenderer->GradientBackgroundOn();

  this->LeftRenderer->SetBackground(this->BackgroundColor);
  this->LeftRenderer->SetBackground2(1,1,1); // White
  //this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);

  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);

  this->LeftStack = vtkSmartPointer<vtkImageStack>::New();
  this->LeftSourceSinkImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->LeftSourceSinkImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->LeftSourceSinkImageSlice->VisibilityOff();
  
  this->LeftSourceSinkImageSliceMapper->SetInputData(this->SourceSinkImageData);

  // Make the pixels sharp instead of blurry when zoomed
  this->LeftSourceSinkImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->LeftSourceSinkImageSlice->SetMapper(this->LeftSourceSinkImageSliceMapper);

  this->LeftStack->AddImage(this->OriginalImageSlice);
  this->LeftStack->AddImage(this->LeftSourceSinkImageSlice);
  
  this->OriginalImageSlice->GetProperty()->SetLayerNumber(0); // 0 = Bottom of the stack
  this->LeftSourceSinkImageSlice->GetProperty()->SetLayerNumber(1); // The source/sink image should be displayed on top of the result image.
  this->LeftStack->SetActiveLayer(1);
  
  this->LeftRenderer->AddViewProp(this->LeftStack);

  // Setup left interactor style
  this->GraphCutStyle = vtkSmartPointer<vtkInteractorStyleScribble>::New();
  this->qvtkWidgetLeft->GetInteractor()->SetInteractorStyle(this->GraphCutStyle);
  this->GraphCutStyle->AddObserver(this->GraphCutStyle->ScribbleEvent,
                                   this, &GraphCutSegmentationWidget::ScribbleEventHandler);
  this->GraphCutStyle->SetCurrentRenderer(this->LeftRenderer);

  this->LeftCamera = new ITKVTKCamera(this->GraphCutStyle, this->LeftRenderer,
                                      this->qvtkWidgetLeft->GetRenderWindow());
}

void GraphCutSegmentationWidget::SetupRightPane()
{
  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer->GradientBackgroundOn();
  this->RightRenderer->SetBackground(this->BackgroundColor);
  this->RightRenderer->SetBackground2(1,1,1); // White
  //this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  // Setup right interactor style
  vtkSmartPointer<vtkInteractorStyleImage> interactorStyleImage =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetInteractor()->SetInteractorStyle(interactorStyleImage);

  this->RightInteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetInteractor()->SetInteractorStyle(this->RightInteractorStyle);
  this->RightInteractorStyle->SetCurrentRenderer(this->RightRenderer);

  this->RightStack = vtkSmartPointer<vtkImageStack>::New();
  this->RightSourceSinkImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->RightSourceSinkImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->RightSourceSinkImageSlice->VisibilityOff();
  
  this->RightSourceSinkImageSliceMapper->SetInputData(this->SourceSinkImageData);

  // Make the pixels sharp instead of blurry when zoomed
  this->RightSourceSinkImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->RightSourceSinkImageSlice->SetMapper(this->RightSourceSinkImageSliceMapper);

  this->RightStack->AddImage(this->ResultSlice);
  this->RightStack->AddImage(this->RightSourceSinkImageSlice);

  this->RightRenderer->AddViewProp(this->RightStack);

  this->RightCamera = new ITKVTKCamera(this->RightInteractorStyle, this->RightRenderer,
                                       this->qvtkWidgetRight->GetRenderWindow());
}

void GraphCutSegmentationWidget::on_actionExit_triggered()
{
  QApplication::quit();
}

void GraphCutSegmentationWidget::SetupCameras()
{
  this->LeftRenderer->ResetCamera();
  this->LeftRenderer->ResetCameraClippingRange();

  this->RightRenderer->ResetCamera();
  this->RightRenderer->ResetCameraClippingRange();

  this->Refresh();
}

void GraphCutSegmentationWidget::on_actionFlipImageVertically_triggered()
{
  this->LeftCamera->FlipVertically();
  this->RightCamera->FlipVertically();
}

void GraphCutSegmentationWidget::on_actionFlipImageHorizontally_triggered()
{
  this->LeftCamera->FlipHorizontally();
  this->RightCamera->FlipHorizontally();
}

void GraphCutSegmentationWidget::on_actionExportSegmentMask_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this,
    "Save Segment Mask Image", "segment_mask.png", "Image Files (*.png *.mha)");

  if(fileName.isEmpty())
  {
    return;
  }
  
  typedef  itk::ImageFileWriter<Mask> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  writer->SetInput(this->GraphCut.GetSegmentMask());
  writer->Update();
}

void GraphCutSegmentationWidget::on_actionOpenImage_triggered()
{
  //std::cout << "actionOpenImage_triggered()" << std::endl;
    //std::cout << "Enter OpenFile()" << std::endl;

  // Get a filename to open
  QString filename = QFileDialog::getOpenFileName(this,
     "Open Image", ".", "Image Files (*.png *.mha)");

  if(filename.isEmpty())
    {
    return;
    }
  OpenFile(filename.toStdString());
}

// Display segmented image with transparent background pixels
void GraphCutSegmentationWidget::slot_SegmentationComplete()
{
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  // Convert the segmentation mask to a binary VTK image
  vtkSmartPointer<vtkImageData> VTKSegmentMask =
    vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKScalarImageToScaledVTKImage(this->GraphCut.GetSegmentMask(), VTKSegmentMask);

  // Convert the image into a VTK image for display
  vtkSmartPointer<vtkImageData> VTKImage =
    vtkSmartPointer<vtkImageData>::New();

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->GraphCut.GetImage(), VTKImage);

  vtkSmartPointer<vtkImageData> VTKMaskedImage =
    vtkSmartPointer<vtkImageData>::New();
  VTKHelpers::MaskImage(VTKImage, VTKSegmentMask, VTKMaskedImage);

  // Remove the old output, set the new output and refresh everything
  //this->ResultActor = vtkSmartPointer<vtkImageActor>::New();
  this->ResultSliceMapper->SetInputData(VTKMaskedImage);

  this->RightSourceSinkImageSlice->VisibilityOn();
  this->ResultSlice->VisibilityOn();
  
  if(!this->AlreadySegmented)
    {
    this->RightRenderer->ResetCamera();
    }
  this->Refresh();

  this->AlreadySegmented = true;
}

float GraphCutSegmentationWidget::ComputeLambda()
{
  // Compute lambda by multiplying the percentage set by the slider by the MaxLambda set in the text box

  double lambdaMax = this->txtLambdaMax->text().toDouble();
  double lambdaPercent = this->sldLambda->value()/100.;
  double lambda = lambdaPercent * lambdaMax;

  return lambda;
}

void GraphCutSegmentationWidget::UpdateLambda()
{
  // Compute lambda and then set the label to this value so the user can see the current setting
  double lambda = ComputeLambda();
  this->lblLambda->setText(QString::number(lambda));
}

void GraphCutSegmentationWidget::sldHistogramBins_valueChanged()
{
  this->GraphCut.SetNumberOfHistogramBins(sldHistogramBins->value());
  //this->lblHistogramBins->setText(QString::number(sldHistogramBins->value())); // This is taken care of by a signal/slot pair setup in QtDesigner
}

void GraphCutSegmentationWidget::on_radForeground_clicked()
{
  this->SelectedPixelSet = &Sources;
  this->GraphCutStyle->SetColorToGreen();
}

void GraphCutSegmentationWidget::on_radBackground_clicked()
{
  this->SelectedPixelSet = &Sinks;
  this->GraphCutStyle->SetColorToRed();
}

void GraphCutSegmentationWidget::on_actionClearAll_activated()
{
  on_actionClearForegroundSelection_activated();
  on_actionClearBackgroundSelection_activated();
}

void GraphCutSegmentationWidget::on_actionClearForegroundSelection_activated()
{
  this->Sources.clear();
  UpdateSelections();
  Refresh();
}

void GraphCutSegmentationWidget::on_actionClearBackgroundSelection_activated()
{
  this->Sinks.clear();
  UpdateSelections();
  Refresh();
}

void GraphCutSegmentationWidget::on_actionSaveForegroundSelection_activated()
{
//   QString directoryName = QFileDialog::getExistingDirectory(this,
//      "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly);
// 
//   std::string foregroundFilename = QDir(directoryName).absoluteFilePath("foreground.png").toStdString();

  QString fileName = QFileDialog::getSaveFileName(this,
    "Save Segment Mask Image", ".", "Image Files (*.png)");

  if(fileName.isEmpty())
  {
    return;
  }
  std::cout << "Writing to " << fileName.toStdString() << std::endl;

  typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
  UnsignedCharScalarImageType::Pointer foregroundImage = UnsignedCharScalarImageType::New();
  foregroundImage->SetRegions(this->ImageRegion);
  foregroundImage->Allocate();
  
  ITKHelpers::IndicesToBinaryImage(this->Sources, foregroundImage);

  ITKHelpers::WriteImage(foregroundImage.GetPointer(), fileName.toStdString());
}


void GraphCutSegmentationWidget::on_actionSaveBackgroundSelection_activated()
{
//   QString directoryName = QFileDialog::getExistingDirectory(this,
//      "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly);
// 
//   std::string backgroundFilename = QDir(directoryName).absoluteFilePath("background.png").toStdString();

  QString fileName = QFileDialog::getSaveFileName(this,
    "Save Segment Mask Image", ".", "Image Files (*.png)");

  if(fileName.isEmpty())
  {
    return;
  }
  
  std::cout << "Writing to " << fileName.toStdString() << std::endl;

  typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
  UnsignedCharScalarImageType::Pointer backgroundImage = UnsignedCharScalarImageType::New();
  backgroundImage->SetRegions(this->ImageRegion);
  backgroundImage->Allocate();
  
  ITKHelpers::IndicesToBinaryImage(this->Sinks, backgroundImage);

  ITKHelpers::WriteImage(backgroundImage.GetPointer(), fileName.toStdString());
}

void GraphCutSegmentationWidget::on_btnCut_clicked()
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

  this->GraphCut.SetSources(this->Sources);
  this->GraphCut.SetSinks(this->Sinks);

  /////////////
  QFuture<void> future = QtConcurrent::run(this->GraphCut, &ImageGraphCut<ImageType>::PerformSegmentation);
  this->FutureWatcher.setFuture(future);

  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  this->ProgressDialog->exec();

}

void GraphCutSegmentationWidget::OpenFile(const std::string& fileName)
{
  // Clear the scribbles
  this->Sources.clear();
  this->Sinks.clear();

  this->ResultSlice->VisibilityOff();
  
  // Read file
  itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();
  reader->SetFileName(fileName);
  reader->Update();

  this->ImageRegion = reader->GetOutput()->GetLargestPossibleRegion();

  this->GraphCut.SetImage(reader->GetOutput());

  // Convert the ITK image to a VTK image and display it
  vtkSmartPointer<vtkImageData> VTKImage = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKImageToVTKRGBImage(reader->GetOutput(), VTKImage);

  this->OriginalImageSliceMapper->SetInputData(VTKImage);

  // Setup the scribble canvas
  VTKHelpers::SetImageSizeToMatch(VTKImage, this->SourceSinkImageData);
  this->SourceSinkImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
  VTKHelpers::MakeImageTransparent(this->SourceSinkImageData);
  
  this->LeftSourceSinkImageSlice->VisibilityOn();
  this->OriginalImageSlice->VisibilityOn();
  
  this->LeftRenderer->ResetCamera();
  this->Refresh();

  // Setup the scribble style
  if(this->radBackground->isChecked())
    {
    this->SelectedPixelSet = &Sinks;
    }
  else
    {
    this->SelectedPixelSet = &Sources;
    }

  this->AlreadySegmented = false;

  this->GraphCutStyle->InitializeTracer(this->LeftSourceSinkImageSlice);
  //std::cout << "Exit OpenFile()" << std::endl;
}

void GraphCutSegmentationWidget::Refresh()
{
  this->qvtkWidgetRight->GetRenderWindow()->Render();
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
  this->qvtkWidgetRight->GetInteractor()->Render();
  this->qvtkWidgetLeft->GetInteractor()->Render();
}

void GraphCutSegmentationWidget::on_actionExportSegmentedImage_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this,
    "Save Segmented Image", "segmented.png", "Image Files (*.png)");

  if(fileName.isEmpty())
  {
    return;
  }

  Mask::Pointer mask = Mask::New();
  ITKHelpers::DeepCopy(this->GraphCut.GetSegmentMask(), mask.GetPointer());

  mask->KeepLargestHole();
  //mask->Invert();

  typedef itk::RGBAPixel<unsigned char> TransparentPixelType;
  typedef itk::Image<TransparentPixelType, 2>  TransparentImageType;

  TransparentImageType::Pointer transparentImage = TransparentImageType::New();
  transparentImage->SetRegions(mask->GetLargestPossibleRegion());
  transparentImage->Allocate();
  
  itk::ImageRegionIterator<TransparentImageType> imageIterator(transparentImage, transparentImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    TransparentImageType::PixelType pixel = imageIterator.Get();

    if(mask->IsHole(imageIterator.GetIndex()))
      {
      pixel.SetRed(this->GraphCut.GetImage()->GetPixel(imageIterator.GetIndex())[0]);
      pixel.SetGreen(this->GraphCut.GetImage()->GetPixel(imageIterator.GetIndex())[1]);
      pixel.SetBlue(this->GraphCut.GetImage()->GetPixel(imageIterator.GetIndex())[2]);
      pixel.SetAlpha(255); // Invisible
      }
    else
      {
      pixel.SetRed(0);
      pixel.SetGreen(0);
      pixel.SetBlue(0);
      pixel.SetAlpha(0); // Visible
      }
    imageIterator.Set(pixel);

    ++imageIterator;
  }

  typedef  itk::ImageFileWriter<TransparentImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName.toStdString());
  writer->SetInput(transparentImage);
  writer->Update();
}

void GraphCutSegmentationWidget::on_actionLoadForeground_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this,
     "Open Image", ".", "PNG Files (*.png)");

  if(filename.isEmpty())
    {
    return;
    }

  typedef  itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.toStdString());
  reader->Update();

  std::vector<itk::Index<2> > pixels = ITKHelpers::GetNonZeroPixels(reader->GetOutput());

  this->Sources=pixels;

  UpdateSelections();  
  std::cout << "Set " << pixels.size() << " new foreground pixels." << std::endl;
}

void GraphCutSegmentationWidget::on_actionLoadBackground_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this,
     "Open Image", ".", "PNG Files (*.png)");

  if(filename.isEmpty())
    {
    return;
    }

  typedef  itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.toStdString());
  reader->Update();

  std::vector<itk::Index<2> > pixels = ITKHelpers::GetNonZeroPixels(reader->GetOutput());

  this->Sinks=pixels;

  UpdateSelections();
  std::cout << "Set " << pixels.size() << " new background pixels." << std::endl;
}

void GraphCutSegmentationWidget::ScribbleEventHandler(vtkObject* caller, long unsigned int eventId, void* callData)
{
  // Dilate the path and mark the path and the dilated pixels as part of the currently selected group
  unsigned int dilateRadius = 2;

  std::vector<itk::Index<2> > thinSelection = ITKVTKHelpers::PointsToPixelList(this->GraphCutStyle->GetSelectionPolyData()->GetPoints());

  std::vector<itk::Index<2> > selection = ITKHelpers::DilatePixelList(thinSelection,
                                                                   this->ImageRegion,
                                                                   dilateRadius);

  this->SelectedPixelSet->insert(this->SelectedPixelSet->end(), selection.begin(), selection.end());

  UpdateSelections();
}

void GraphCutSegmentationWidget::UpdateSelections()
{
  // First, clear the image
  VTKHelpers::MakeImageTransparent(this->SourceSinkImageData);

  unsigned char green[3] = {0, 255, 0};
  unsigned char red[3] = {255, 0, 0};

  ITKVTKHelpers::SetPixels(this->SourceSinkImageData, this->Sources, green);
  ITKVTKHelpers::SetPixels(this->SourceSinkImageData, this->Sinks, red);

  this->SourceSinkImageData->Modified();

  std::cout << this->Sources.size() << " sources." << std::endl;
  std::cout << this->Sinks.size() << " sinks." << std::endl;

  this->Refresh();
}

void GraphCutSegmentationWidget::closeEvent(QCloseEvent *)
{
  QApplication::quit();
}

void GraphCutSegmentationWidget::on_btnHideStrokesLeft_clicked()
{
  this->LeftSourceSinkImageSlice->VisibilityOff();
  Refresh();
}

void GraphCutSegmentationWidget::on_btnShowStrokesLeft_clicked()
{
  this->LeftSourceSinkImageSlice->VisibilityOn();
  Refresh();
}

void GraphCutSegmentationWidget::on_btnHideStrokesRight_clicked()
{
  this->RightSourceSinkImageSlice->VisibilityOff();
  Refresh();
}

void GraphCutSegmentationWidget::on_btnShowStrokesRight_clicked()
{
  this->RightSourceSinkImageSlice->VisibilityOn();
  Refresh();
}

void GraphCutSegmentationWidget::on_actionExportScreenshotLeft_triggered()
{
  QString filename = QFileDialog::getSaveFileName(this, "Save PNG Screenshot", "", "PNG Files (*.png)");

  if(filename.isEmpty())
    {
    return;
    }

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->qvtkWidgetLeft->GetRenderWindow());
  //windowToImageFilter->SetMagnification(3);
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer =
    vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(filename.toStdString().c_str());
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();
}
