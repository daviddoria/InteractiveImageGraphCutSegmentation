#ifndef FILEMENUFORM_H
#define FILEMENUFORM_H

// Qt
#include "ui_form.h"
#include "types.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"

// Custom
#include "vtkGraphCutInteractorStyle.h"
#include "ImageGraphCutBase.h"
#include "ImageGraphCut.h"

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
    void radForeground_clicked();
    void radBackground_clicked();
    void sldHistogramBins_valueChanged();
    void UpdateLambda();

protected:
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
//void ITKImagetoVTKImage(GrayscaleImageType::Pointer image, vtkImageData* outputImage);

#endif
