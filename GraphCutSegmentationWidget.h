/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

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

#ifndef GraphCutSegmentationWidget_H
#define GraphCutSegmentationWidget_H

#include "ui_GraphCutSegmentationWidget.h"

// Qt
#include <QFutureWatcher>
#include <QProgressDialog>

// Custom
#include "ImageGraphCutSegmentation/ImageGraphCut.h"

// Submodules
#include "ScribbleInteractorStyle/vtkInteractorStyleScribble.h"
#include "ITKVTKCamera/ITKVTKCamera.h"

// VTK
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkImageStack;

class GraphCutSegmentationWidget : public QMainWindow, private Ui::GraphCutSegmentationWidget
{
Q_OBJECT
public:
  GraphCutSegmentationWidget(QWidget *parent = 0);
  GraphCutSegmentationWidget(const std::string& fileName);

public slots:
  /** Menu items*/
  // View menu
  void on_actionFlipImageVertically_triggered();
  void on_actionFlipImageHorizontally_triggered();

  // Export menu
  void on_actionExportSegmentedImage_triggered();
  void on_actionExportSegmentMask_triggered();
  void on_actionExportScreenshotLeft_triggered();

  // File menu
  void on_actionExit_triggered();
  void on_actionOpenImage_triggered();

  void on_actionLoadForeground_triggered();
  void on_actionLoadBackground_triggered();
  
  /** Buttons, radio buttons, and sliders*/
  void on_actionSaveForegroundSelection_activated();
  void on_actionSaveBackgroundSelection_activated();
  
  void on_actionClearBackgroundSelection_activated();
  void on_actionClearForegroundSelection_activated();
  void on_actionClearAll_activated();

  void on_btnCut_clicked();
  void on_radForeground_clicked();
  void on_radBackground_clicked();

  void on_btnHideStrokesLeft_clicked();
  void on_btnShowStrokesLeft_clicked();
  void on_btnHideStrokesRight_clicked();
  void on_btnShowStrokesRight_clicked();
  
  void sldHistogramBins_valueChanged();

  void slot_SegmentationComplete();

  /** Setting lambda must be handled specially because we need to multiply the
   *  percentage set by the slider by the MaxLambda set in the text box
   */
  void UpdateLambda();

  /** Open the specified file as a greyscale or color image,
   *  depending on which type the user has specified through the file menu.
   */
  void OpenFile(const std::string& fileName);

protected:

  void ScribbleEventHandler(vtkObject* caller, long unsigned int eventId, void* callData);
  
  /** A constructor that can be used by all other constructors. */
  void SharedConstructor();

  /** Compute lambda by multiplying the percentage set by the slider by the MaxLambda set in the text box. */
  float ComputeLambda();

  /** Our scribble interactor style */
  vtkSmartPointer<vtkInteractorStyleScribble> GraphCutStyle;

  /** The interactor style for the resulting segmented image. */
  vtkSmartPointer<vtkInteractorStyleImage> RightInteractorStyle;
  
  /** The input and output image actors */
  vtkSmartPointer<vtkImageSlice> OriginalImageSlice;
  vtkSmartPointer<vtkImageSlice> ResultSlice;

  vtkSmartPointer<vtkImageSliceMapper> OriginalImageSliceMapper;
  vtkSmartPointer<vtkImageSliceMapper> ResultSliceMapper;
  
  /** The renderers */
  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkRenderer> RightRenderer;

  /** Refresh both renderers and render windows */
  void Refresh();

  /** The main segmentation class.  */
  ImageGraphCut GraphCut;

  /** Allows the background color to be changed*/
  double BackgroundColor[3];

  void SetupCameras();
  
  /** We set this when the image is opeend. We sometimes need to know how big the image is.*/
  itk::ImageRegion<2> ImageRegion;

  QFutureWatcher<void> FutureWatcher;
  QProgressDialog* ProgressDialog;

  bool AlreadySegmented;

  vtkSmartPointer<vtkImageStack> LeftStack;
  vtkSmartPointer<vtkImageStack> RightStack;

  /** Both panes - This data can be used by both the Left and Right SourceSinkImageSliceMapper */
  vtkSmartPointer<vtkImageData> SourceSinkImageData;

  vtkSmartPointer<vtkImageSliceMapper> LeftSourceSinkImageSliceMapper;
  vtkSmartPointer<vtkImageSlice> LeftSourceSinkImageSlice;

  vtkSmartPointer<vtkImageSliceMapper> RightSourceSinkImageSliceMapper;
  vtkSmartPointer<vtkImageSlice> RightSourceSinkImageSlice;

  void SetupLeftPane();
  void SetupRightPane();

  /** This function setups up things that are shared by both panes. */
  void SetupBothPanes();

  typedef std::vector<itk::Index<2> > VectorOfPixels;
  VectorOfPixels Sources;
  VectorOfPixels Sinks;
  VectorOfPixels* SelectedPixelSet;
  
  void UpdateSelections();

  void closeEvent(QCloseEvent *);

  ITKVTKCamera* LeftCamera = nullptr;
  ITKVTKCamera* RightCamera = nullptr;
};

#endif
