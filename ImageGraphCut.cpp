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

#include "ImageGraphCut.h"

#include "itkImageRegionIterator.h"
#include "itkShapedNeighborhoodIterator.h"

#include <cmath>

#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

template <typename TImageType>
ImageGraphCut<TImageType>::ImageGraphCut(typename TImageType::Pointer image)
{
  this->Image = TImageType::New();
  this->Image->Graft(image);

  this->SegmentMask = GrayscaleImageType::New();
  this->SegmentMask->SetRegions(this->Image->GetLargestPossibleRegion());
  this->SegmentMask->Allocate();

  this->NodeImage = NodeImageType::New();
  this->NodeImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->NodeImage->Allocate();

  this->Lambda = 0.01;
  this->NumberOfHistogramBins = 10;

  this->ForegroundHistogram = NULL;
  this->BackgroundHistogram = NULL;

  this->ForegroundSample = SampleType::New();
  this->BackgroundSample = SampleType::New();

  ForegroundHistogramFilter = SampleToHistogramFilterType::New();
  this->BackgroundHistogramFilter = SampleToHistogramFilterType::New();

}

template <typename TImageType>
void ImageGraphCut<TImageType>::CutGraph()
{
  // Compute max-flow
  this->Graph->maxflow();

  itk::ImageRegionConstIterator<NodeImageType> nodeImageIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  nodeImageIterator.GoToBegin();

  GrayscalePixelType sinkPixel;
  sinkPixel[0] = 0;

  GrayscalePixelType sourcePixel;
  sourcePixel[0] = 255;

  while(!nodeImageIterator.IsAtEnd())
    {
    if(this->Graph->what_segment(nodeImageIterator.Get()) == GraphType::SOURCE)
      {
      this->SegmentMask->SetPixel(nodeImageIterator.GetIndex(), sourcePixel);
      }
    else if(this->Graph->what_segment(nodeImageIterator.Get()) == GraphType::SINK)
      {
      this->SegmentMask->SetPixel(nodeImageIterator.GetIndex(), sinkPixel);
      }
    ++nodeImageIterator;
    }

  delete this->Graph;
}

template <typename TImageType>
void ImageGraphCut<TImageType>::PerformSegmentation()
{
  itk::ImageRegionIterator<NodeImageType> nodeImageIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  nodeImageIterator.GoToBegin();

  while(!nodeImageIterator.IsAtEnd())
    {
    nodeImageIterator.Set(NULL);
    ++nodeImageIterator;
    }

  itk::ImageRegionIterator<GrayscaleImageType> segmentMaskImageIterator(this->SegmentMask, this->SegmentMask->GetLargestPossibleRegion());
  segmentMaskImageIterator.GoToBegin();

  while(!segmentMaskImageIterator.IsAtEnd())
    {
    MaskImageType::PixelType empty;
    empty[0] = 0;
    segmentMaskImageIterator.Set(empty);
    ++segmentMaskImageIterator;
    }

  this->CreateGraph();
  this->CutGraph();
}

template <typename TImageType>
void ImageGraphCut<TImageType>::CreateSamples()
{
  // Setup the histogram size
  typename SampleToHistogramFilterType::HistogramSizeType histogramSize(TImageType::PixelType::GetNumberOfComponents());
  histogramSize.Fill(this->NumberOfHistogramBins);

  // Create foreground samples and histogram
  this->ForegroundSample->Clear();
  for(unsigned int i = 0; i < this->Sources.size(); i++)
    {
    this->ForegroundSample->PushBack(this->Image->GetPixel(this->Sources[i]));
    }

  this->ForegroundHistogramFilter->SetHistogramSize(histogramSize);
  this->ForegroundHistogramFilter->SetInput(this->ForegroundSample);
  this->ForegroundHistogramFilter->Modified();
  this->ForegroundHistogramFilter->Update();

  this->ForegroundHistogram = this->ForegroundHistogramFilter->GetOutput();

  // Create background samples and histogram
  this->BackgroundSample->Clear();
  for(unsigned int i = 0; i < this->Sinks.size(); i++)
    {
    this->BackgroundSample->PushBack(this->Image->GetPixel(this->Sinks[i]));
    }

  this->BackgroundHistogramFilter->SetHistogramSize(histogramSize);
  this->BackgroundHistogramFilter->SetInput(this->BackgroundSample);
  this->BackgroundHistogramFilter->Modified();
  this->BackgroundHistogramFilter->Update();

  this->BackgroundHistogram = BackgroundHistogramFilter->GetOutput();

}

template <typename TImageType>
void ImageGraphCut<TImageType>::CreateGraph()
{
  // Form the graph
  this->Graph = new GraphType;

  // Add all of the nodes to the graph and store their IDs in a "node image"
  itk::ImageRegionIterator<NodeImageType> nodeImageIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  nodeImageIterator.GoToBegin();

  while(!nodeImageIterator.IsAtEnd())
    {
    nodeImageIterator.Set(this->Graph->add_node());
    ++nodeImageIterator;
    }

  double sigma = this->ComputeNoise();

  // Set n-edges (links between image nodes)
  itk::Size<2> radius;
  radius[0] = 1;
  radius[1] = 1;

  typedef itk::ShapedNeighborhoodIterator<TImageType> IteratorType;

  std::vector<typename IteratorType::OffsetType> neighbors;
  typename IteratorType::OffsetType bottom = {{0,1}};
  neighbors.push_back(bottom);
  typename IteratorType::OffsetType right = {{1,0}};
  neighbors.push_back(right);

  typename IteratorType::OffsetType center = {{0,0}};

  IteratorType iterator(radius, this->Image, this->Image->GetLargestPossibleRegion());
  iterator.ClearActiveList();
  iterator.ActivateOffset(bottom);
  iterator.ActivateOffset(right);
  iterator.ActivateOffset(center);

  for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
    typename TImageType::PixelType centerPixel = iterator.GetPixel(center);

    for(unsigned int i = 0; i < neighbors.size(); i++)
      {
      bool valid;
      iterator.GetPixel(neighbors[i], valid);
      if(!valid)
        {
        continue;
        }
      typename TImageType::PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

      float pixelDifference = PixelDifference(centerPixel, neighborPixel);
      float weight = exp(-pow(pixelDifference,2)/(2.0*sigma*sigma));
      assert(weight >= 0);

      void* node1 = this->NodeImage->GetPixel(iterator.GetIndex(center));
      void* node2 = this->NodeImage->GetPixel(iterator.GetIndex(neighbors[i]));
      this->Graph->add_edge(node1, node2, weight, weight);
      }
    }

  // Set t-edges (links from image nodes to virtual background and virtual foreground node)

  CreateSamples();

  itk::ImageRegionIterator<TImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<NodeImageType> nodeIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  nodeIterator.GoToBegin();

  float minNonZeroSinkHistogramValue = FindMinNonZeroHistogramValue(this->BackgroundHistogram);
  float minNonZeroSourceHistogramValue = FindMinNonZeroHistogramValue(this->ForegroundHistogram);

  minNonZeroSinkHistogramValue /= static_cast<double>(this->BackgroundHistogram->GetTotalFrequency());
  minNonZeroSourceHistogramValue /= static_cast<double>(this->ForegroundHistogram->GetTotalFrequency());

  while(!imageIterator.IsAtEnd())
    {
    typename TImageType::PixelType pixel = imageIterator.Get();

    HistogramType::MeasurementVectorType measurementVector(pixel.Size());
    for(unsigned int i = 0; i < pixel.Size(); i++)
      {
      measurementVector[i] = pixel[i];
      }

    float sinkHistogramValue = this->BackgroundHistogram->GetFrequency(this->BackgroundHistogram->GetIndex(measurementVector));
    float sourceHistogramValue = this->ForegroundHistogram->GetFrequency(this->ForegroundHistogram->GetIndex(measurementVector));

    // Normalize the histograms to have area = 1
    sinkHistogramValue /= this->BackgroundHistogram->GetTotalFrequency();
    sourceHistogramValue /= this->ForegroundHistogram->GetTotalFrequency();

    if(sinkHistogramValue <= 0)
      {
      sinkHistogramValue = minNonZeroSinkHistogramValue;
      }
    if(sourceHistogramValue <= 0)
      {
      sourceHistogramValue = minNonZeroSourceHistogramValue;
      }

    //assert(sinkHistogramValue > 0 && sinkHistogramValue < 1);
    if(!(sinkHistogramValue > 0 && sinkHistogramValue < 1))
      {
      std::cerr << "Sink histogram value: " << sinkHistogramValue << std::endl;
      exit(-1);
      }
    assert(sourceHistogramValue > 0 && sourceHistogramValue < 1);

    this->Graph->add_tweights(nodeIterator.Get(),
                              -this->Lambda*log(sinkHistogramValue),
                              -this->Lambda*log(sourceHistogramValue)); // log() is the natural log
    ++imageIterator;
    ++nodeIterator;
    }

  // Set infinite source weights
  for(unsigned int i = 0; i < this->Sources.size(); i++)
    {
    this->Graph->add_tweights(this->NodeImage->GetPixel(this->Sources[i]),std::numeric_limits<float>::infinity(),0);
    }

  // Set infinite sink weights
  for(unsigned int i = 0; i < this->Sinks.size(); i++)
    {
    this->Graph->add_tweights(this->NodeImage->GetPixel(this->Sinks[i]),0,std::numeric_limits<float>::infinity());
    }
}

template <typename TImageType>
template <typename TPixelType>
float ImageGraphCut<TImageType>::PixelDifference(TPixelType a, TPixelType b)
{
  float difference = 0;
  for(unsigned int i = 0; i < TPixelType::GetNumberOfComponents(); i++)
    {
    difference += pow(a[i] - b[i],2);
    }
  return sqrt(difference);
}

template <typename TImageType>
float ImageGraphCut<TImageType>::FindMinNonZeroHistogramValue(const HistogramType* histogram)
{
  assert(histogram);

  HistogramType::ConstIterator histogramIterator = histogram->Begin();

  // Initialize
  unsigned int minNonZeroFrequency;

  while(histogramIterator != histogram->End())
    {
    if(histogramIterator.GetFrequency() > 0)
      {
      minNonZeroFrequency = histogramIterator.GetFrequency();
      }
    ++histogramIterator ;
    }

  // Compute
  histogramIterator = histogram->Begin();
  while(histogramIterator != histogram->End())
    {
    if((histogramIterator.GetFrequency() > 0) && (histogramIterator.GetFrequency() < minNonZeroFrequency))
      {
      minNonZeroFrequency = histogramIterator.GetFrequency();
      }
    ++histogramIterator ;
    }

  return minNonZeroFrequency;
}

template <typename TImageType>
double ImageGraphCut<TImageType>::ComputeNoise()
{
  itk::Size<2> radius;
  radius[0] = 1;
  radius[1] = 1;

  typedef itk::ShapedNeighborhoodIterator<TImageType> IteratorType;

  std::vector<typename IteratorType::OffsetType> neighbors;
  typename IteratorType::OffsetType bottom = {{0,1}};
  neighbors.push_back(bottom);
  typename IteratorType::OffsetType right = {{1,0}};
  neighbors.push_back(right);

  typename IteratorType::OffsetType center = {{0,0}};

  IteratorType iterator(radius, this->Image, this->Image->GetLargestPossibleRegion());
  iterator.ClearActiveList();
  iterator.ActivateOffset(bottom);
  iterator.ActivateOffset(right);
  iterator.ActivateOffset(center);

  double sigma = 0.0;
  int sum = 0;

  for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
    typename TImageType::PixelType centerPixel = iterator.GetPixel(center);

    for(unsigned int i = 0; i < neighbors.size(); i++)
      {
      bool valid;
      iterator.GetPixel(neighbors[i], valid);
      if(!valid)
        {
        continue;
        }
      typename TImageType::PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

      float colorDifference = PixelDifference(centerPixel, neighborPixel);
      sigma += colorDifference;
      sum++;
      }

    }

  sigma /= static_cast<double>(sum);

  return sigma;
}

// Explicit template instantiations
template class ImageGraphCut<ColorImageType>;
template class ImageGraphCut<GrayscaleImageType>;