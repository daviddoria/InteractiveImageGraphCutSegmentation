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
#include "types.h"
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