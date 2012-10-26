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

#ifndef IMAGEGRAPHCUT_H
#define IMAGEGRAPHCUT_H

// VTK
#include <vtkSmartPointer.h>
class vtkPolyData;

#include "Mask/Mask.h"

// ITK
#include "itkImage.h"
#include "itkSampleToHistogramFilter.h"
#include "itkHistogram.h"
#include "itkListSample.h"

// STL
#include <vector>

// Custom
#include "Types.h"

// Kolmogorov's code
#include "ImageGraphCutSegmentation/Kolmogorov/graph.h"
typedef Graph GraphType;

class ImageGraphCut
{
public:

  /** This is a special type to keep track of the graph node labels. */
  typedef itk::Image<void*, 2> NodeImageType;

  /** The type of the histograms. */
  typedef itk::Statistics::Histogram< float,
          itk::Statistics::DenseFrequencyContainer2 > HistogramType;

  /** The type of a list of pixels/indexes. */
  typedef std::vector<itk::Index<2> > IndexContainer;

  /** Several initializations are done here. */
  void SetImage(ImageType* const image);

  /** Get the image that we are segmenting. */
  ImageType* GetImage();

  /** Create and cut the graph (The main driver function). */
  void PerformSegmentation();

  /** Return a list of the selected (via scribbling) pixels. */
  IndexContainer GetSources();
  IndexContainer GetSinks();

  /** Set the selected pixels. */
  void SetSources(vtkPolyData* const sources);
  void SetSinks(vtkPolyData* const sinks);

  /** Set the selected pixels. */
  void SetSources(const IndexContainer& sources);
  void SetSinks(const IndexContainer& sinks);

  /** Get the output of the segmentation. */
  Mask* GetSegmentMask();

  /** Set the weight between the regional and boundary terms. */
  void SetLambda(const float);

  /** Set the number of bins per dimension of the foreground and background histograms. */
  void SetNumberOfHistogramBins(const int);

protected:

  /** A Kolmogorov graph object */
  GraphType* Graph;

  /** The output segmentation */
  Mask::Pointer SegmentMask;

  /** User specified foreground points */
  IndexContainer Sources;

  /** User specified background points */
  IndexContainer Sinks;

  /** The weighting between unary and binary terms */
  float Lambda;

  /** The number of bins per dimension of the foreground and background histograms */
  int NumberOfHistogramBins;

  /** An image which keeps tracks of the mapping between pixel index and graph node id */
  NodeImageType::Pointer NodeImage;

  /** Determine if a number is NaN */
  bool IsNaN(const double a);

  /** The relative weight of the RGB channels (assumed to be the first 3 channels) if the image has more than 3 channels. */
  float RGBWeight;

  // Typedefs
  typedef itk::Statistics::ListSample<PixelType> SampleType;
  typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;

  /** Create the histograms from the users selections */
  void CreateSamples();

  /** Estimate the "camera noise" */
  double ComputeNoise();

  /** Create a Kolmogorov graph structure from the image and selections */
  void CreateGraph();

  /** Perform the s-t min cut */
  void CutGraph();

  /** Compute the difference between two pixels. */
  float PixelDifference(const PixelType& a, const PixelType& b);

  /** The ITK data structure for storing the values that we will compute the histogram of. */
  SampleType::Pointer ForegroundSample;
  SampleType::Pointer BackgroundSample;

  /** The histograms. */
  const HistogramType* ForegroundHistogram;
  const HistogramType* BackgroundHistogram;

  /** ITK filters to create histograms. */
  SampleToHistogramFilterType::Pointer ForegroundHistogramFilter;
  SampleToHistogramFilterType::Pointer BackgroundHistogramFilter;

  /** The image to be segmented */
  ImageType::Pointer Image;

};

#endif
