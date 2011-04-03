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
#include "InnerWidget.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);
  connect( this->actionOpen_Color_Image, SIGNAL( triggered() ), this, SLOT(actionOpen_Color_Image_triggered()) );
  connect( this->actionOpen_Grayscale_Image, SIGNAL( triggered() ), this, SLOT(actionOpen_Grayscale_Image_triggered()) );

  //connect( this->actionFlip_Image, SIGNAL( triggered() ), this, SLOT(actionFlip_Image_triggered()));
  //connect( this->actionSave_Segmentation, SIGNAL( triggered() ), this, SLOT(actionSave_Segmentation_triggered()));

}


#if 0
void Form::actionFlip_Image_triggered()
{
  this->CameraUp[1] *= -1;
  this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->Refresh();
}
#endif

#if 0
void Form::actionSave_Segmentation_triggered()
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

void MainWindow::actionOpen_Grayscale_Image_triggered()
{
  // Instantiate everything to use grayscale images and grayscale histograms
  std::cout << "Grayscale form." << std::endl;
  InnerWidget<GrayscaleImageType>* insideForm = new InnerWidget<GrayscaleImageType>(this);
  this->setCentralWidget(insideForm);
  insideForm->OpenFile();
}

void MainWindow::actionOpen_Color_Image_triggered()
{
  // Instantiate everything to use color images and color histograms
  std::cout << "Color form." << std::endl;
  InnerWidget<ColorImageType>* insideForm = new InnerWidget<ColorImageType>(this);
  this->setCentralWidget(insideForm);
  insideForm->OpenFile();
}
