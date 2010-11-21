#ifndef IMAGEGRAPHCUTBASE_H
#define IMAGEGRAPHCUTBASE_H

// VTK
class vtkPolyData;
#include <vtkSmartPointer.h>

// ITK
#include <itkImageBase.h>

// STL
#include <vector>

// Custom
#include "types.h"

// Kolmogorov
#include "graph.h"
typedef Graph GraphType;

class ImageGraphCutBase
{
public:
  ImageGraphCutBase();

  // Type dependent
  //virtual void ClearSelections() = 0;

  // Non type dependent
  std::vector<itk::Index<2> > GetSources();
  std::vector<itk::Index<2> > GetSinks();

  void SetSources(vtkPolyData* sources);
  void SetSinks(vtkPolyData* sinks);

  MaskImageType::Pointer GetSegmentMask();
  void SetLambda(float);
  void SetNumberOfHistogramBins(int);
  virtual void PerformSegmentation() = 0;

protected:
  GraphType* Graph;

  MaskImageType::Pointer SegmentMask; // The output segmentation

  std::vector<itk::Index<2> > Sources; // User specified foreground points
  std::vector<itk::Index<2> > Sinks; // User specified background points

  float Lambda; // The weighting between unary and binary terms
  int NumberOfHistogramBins;

  NodeImageType::Pointer NodeImage;

  bool IsNaN(const double a);

  virtual void CreateGraph() = 0;
  virtual void CutGraph() = 0;
};

#endif