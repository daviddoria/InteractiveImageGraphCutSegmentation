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

#include "Types.h"

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

  void SetSources(std::vector<itk::Index<2> > sources);
  void SetSinks(std::vector<itk::Index<2> > sinks);

  // Get the output of the segmentation
  MaskImageType::Pointer GetSegmentMask();

  // Set the weight between the regional and boundary terms
  void SetLambda(float);

  // Set the weight of the RGB components of the pixels vs the rest of the components
  void SetRGBWeight(float);
  
  // Set the number of bins per dimension of the foreground and background histograms
  void SetNumberOfHistogramBins(int);

  // The main driver function
  virtual void PerformSegmentation() = 0;

  // The dimensionality of the loaded image pixels
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

  // The dimensionality of the loaded image pixels
  unsigned int PixelDimensionality;

  float RGBWeight;
};

#endif