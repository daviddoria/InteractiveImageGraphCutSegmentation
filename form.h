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

#ifndef FILEMENUFORM_H
#define FILEMENUFORM_H

// Qt
#include "ui_form.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"

// Custom
#include "vtkGraphCutInteractorStyle.h"
#include "ImageGraphCutBase.h"
#include "ImageGraphCut.h"
#include "ProgressThread.h"

// VTK
#include <vtkSmartPointer.h>

class vtkImageActor;
class vtkRenderer;
class vtkImageData;

class Form : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
public:
    Form(QWidget *parent = 0);

public slots:
    void actionOpen_Color_Image_triggered();
    void actionOpen_Grayscale_Image_triggered();
    void btnClearSelections_clicked();
    void btnCut_clicked();
    void btnSave_clicked();
    void radForeground_clicked();
    void radBackground_clicked();
    void sldHistogramBins_valueChanged();
    void UpdateLambda();
    void StartProgressSlot();
    void StopProgressSlot();

protected:
  CProgressThread ProgressThread;

  float ComputeLambda();

  template<typename TImageType>
  void OpenFile();

  vtkSmartPointer<vtkGraphCutInteractorStyle> GraphCutStyle;

  vtkSmartPointer<vtkImageActor> OriginalImageActor;
  vtkSmartPointer<vtkImageActor> ResultActor;

  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkRenderer> RightRenderer;

  void Refresh();

  ImageGraphCutBase* GraphCut;

};

// Helpers
template <typename TImageType>
void ITKImagetoVTKImage(typename TImageType::Pointer image, vtkImageData* outputImage);

#endif
