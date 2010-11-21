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

// ITK
#include <itkImage.h>

// STL
#include <vector>

// Custom
#include "ImageGraphCutBase.h"

// Kolmogorov's code
#include "graph.h"
typedef Graph GraphType;

template <typename TImageType>
class ImageGraphCut : public ImageGraphCutBase
{
public:
  ImageGraphCut(typename TImageType::Pointer image);

  void SetSources(vtkPolyData* sources);
  void SetSinks(vtkPolyData* sinks);

  void PerformSegmentation();

protected:

  // Typedefs
  typedef itk::Statistics::ListSample<typename TImageType::PixelType> SampleType;
  typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;

  void CreateSamples();
  double ComputeNoise();

  void CreateGraph();
  void CutGraph();

  template <typename TPixelType>
  float PixelDifference(TPixelType, TPixelType);

  float FindMinNonZeroHistogramValue(const HistogramType* histogram);

  // Member variables
  typename SampleType::Pointer ForegroundSample;
  typename SampleType::Pointer BackgroundSample;

  const HistogramType* ForegroundHistogram;
  const HistogramType* BackgroundHistogram;

  typename SampleToHistogramFilterType::Pointer ForegroundHistogramFilter;
  typename SampleToHistogramFilterType::Pointer BackgroundHistogramFilter;

  typename TImageType::Pointer Image;

};

#endif