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

/* This is the main GUI class of this project. It is a QMainWindow
 * so that we can use a File menu. It contains an instance of our main functional
 * class ImageGraphCutBase and our custom scribble interactor style vtkGraphCutInteractorStyle.
 * It also contains a CProgressThread so that we can display a progress bar in marquee
 * mode during long computations.
*/

#ifndef FILEMENUFORM_H
#define FILEMENUFORM_H

// Qt
#include "ui_form.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "vtkGraphCutInteractorStyle.h"
#include "ImageGraphCutBase.h"
#include "ImageGraphCut.h"
#include "ProgressThread.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

// Forward declarations
class vtkImageActor;
class vtkRenderer;
class vtkImageData;

class Form : public QMainWindow, private Ui::MainWindow
{
Q_OBJECT
public:
  Form(QWidget *parent = 0);

public slots:
  // Menu items
  void actionOpen_Color_Image_triggered();
  void actionOpen_Grayscale_Image_triggered();
  void actionOpen_RGBDI_Image_triggered();
  void actionFlip_Image_triggered();
  void actionSave_Segmentation_triggered();

  // Buttons, radio buttons, and sliders
  void btnClearSelections_clicked();
  void btnSaveSelections_clicked();
  void btnCut_clicked();
  void radForeground_clicked();
  void radBackground_clicked();
  void sldHistogramBins_valueChanged();

  // Setting lambda must be handled specially because we need to multiply the percentage set by the slider by the MaxLambda set in the text box
  void UpdateLambda();

  // These slots handle running the progress bar while the computations are done in a separate thread.
  void StartProgressSlot();
  void StopProgressSlot();

protected:

  // A class to do the main computations in a separate thread so we can display a marquee progress bar.
  CProgressThread ProgressThread;

  // Compute lambda by multiplying the percentage set by the slider by the MaxLambda set in the text box.
  float ComputeLambda();

  // Use a QFileDialog to get a filename, then open the specified file as a greyscale or color image, depending on which type the user has specified through the file menu.
  template<typename TImageType>
  void OpenFile();

  // Our scribble interactor style
  vtkSmartPointer<vtkGraphCutInteractorStyle> GraphCutStyle;

  // The input and output image actors
  vtkSmartPointer<vtkImageActor> OriginalImageActor;
  vtkSmartPointer<vtkImageActor> ResultActor;

  // The renderers
  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkRenderer> RightRenderer;

  // Refresh both renderers and render windows
  void Refresh();

  // The main segmentation class. This will be instantiated as a ImageGraphCut after the user selects whether to open a color or grayscale image.
  ImageGraphCutBase* GraphCut;

  // Allows the background color to be changed
  double BackgroundColor[3];

  // Allows the image to be flipped so that it is "right side up"
  double CameraUp[3];

  // We set this when the image is opeend. We sometimes need to know how big the image is.
  itk::ImageRegion<2> ImageRegion;

};

//////// Helpers ////////

// Convert an ITK image to a VTK image for display
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
    for(unsigned int component = 0; component < TImageType::PixelType::GetNumberOfComponents(); component++)
      {
      pixel[component] = static_cast<unsigned char>(imageIterator.Get()[component]);
      }

    ++imageIterator;
    }
}

void MaskImage(vtkSmartPointer<vtkImageData> VTKImage, vtkSmartPointer<vtkImageData> VTKSegmentMask, vtkSmartPointer<vtkImageData> VTKMaskedImage);


#endif
