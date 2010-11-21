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

#ifndef IMAGEGRAPHCUTBASE_H
#define IMAGEGRAPHCUTBASE_H

// VTK
class vtkPolyData;
#include <vtkSmartPointer.h>

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkSampleToHistogramFilter.h"
#include "itkHistogram.h"
#include "itkListSample.h"

// STL
#include <vector>

// Kolmogorov
#include "graph.h"
typedef Graph GraphType;

// Typedefs
typedef itk::CovariantVector<float,3> ColorPixelType;
typedef itk::CovariantVector<float,1> GrayscalePixelType;

typedef itk::Image<ColorPixelType, 2> ColorImageType;
typedef itk::Image<GrayscalePixelType, 2> GrayscaleImageType;

typedef itk::Image<GrayscalePixelType, 2> MaskImageType;

typedef itk::Image<void*, 2> NodeImageType;

typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 > HistogramType;

class ImageGraphCutBase
{
public:
  ImageGraphCutBase();

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