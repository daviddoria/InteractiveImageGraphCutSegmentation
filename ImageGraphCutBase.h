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

// All images are stored internally as float pixels
typedef itk::CovariantVector<float,3> ColorPixelType;
typedef itk::CovariantVector<float,1> GrayscalePixelType;
typedef itk::CovariantVector<float,5> RGBDIPixelType;

typedef itk::Image<ColorPixelType, 2> ColorImageType;
typedef itk::Image<GrayscalePixelType, 2> GrayscaleImageType;
typedef itk::Image<RGBDIPixelType, 2> RGBDIImageType;

typedef itk::Image<GrayscalePixelType, 2> MaskImageType;

// For writing images, we need to first convert to actual unsigned char images
typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;

// This is a special type to keep track of the graph node labels
typedef itk::Image<void*, 2> NodeImageType;

typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 > HistogramType;

class ImageGraphCutBase
{
public:
  ImageGraphCutBase();

  // Return a list of the selected (via scribbling) pixels
  std::vector<itk::Index<2> > GetSources();
  std::vector<itk::Index<2> > GetSinks();

  // Set the selected (via scribbling) pixels
  void SetSources(vtkPolyData* sources);
  void SetSinks(vtkPolyData* sinks);

  // Get the output of the segmentation
  MaskImageType::Pointer GetSegmentMask();

  // Set the weight between the regional and boundary terms
  void SetLambda(float);

  // Set the number of bins per dimension of the foreground and background histograms
  void SetNumberOfHistogramBins(int);

  // The main driver function
  virtual void PerformSegmentation() = 0;

  unsigned int GetPixelDimensionality();

protected:

  // A Kolmogorov graph object
  GraphType* Graph;

  // The output segmentation
  MaskImageType::Pointer SegmentMask;

  // User specified foreground points
  std::vector<itk::Index<2> > Sources;

  // User specified background points
  std::vector<itk::Index<2> > Sinks;

  // The weighting between unary and binary terms
  float Lambda;

  // The number of bins per dimension of the foreground and background histograms
  int NumberOfHistogramBins;

  // An image which keeps tracks of the mapping between pixel index and graph node id
  NodeImageType::Pointer NodeImage;

  // Determine if a number is NaN
  bool IsNaN(const double a);

  // Create the graph structure and weight the edges
  virtual void CreateGraph() = 0;

  // Perform an s-t min cut on the graph
  virtual void CutGraph() = 0;

  unsigned int PixelDimensionality;
};

#endif