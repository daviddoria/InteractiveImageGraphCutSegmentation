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

// Submodules
#include "Mask/ITKHelpers/Helpers/Helpers.h"
#include "Mask/ITKHelpers/ITKHelpers.h"

// ITK
#include "itkImageRegionIterator.h"
#include "itkShapedNeighborhoodIterator.h"
#include "itkMaskImageFilter.h"

// STL
#include <cmath>

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

// Qt
#include <QMessageBox>

void ImageGraphCut::SetImage(ImageType* const image)
{

  this->Image = ImageType::New();
  //this->Image->Graft(image);
  ITKHelpers::DeepCopy(image, this->Image.GetPointer());

  // Setup the output (mask) image
  //this->SegmentMask = GrayscaleImageType::New();
  this->SegmentMask = Mask::New();
  this->SegmentMask->SetRegions(this->Image->GetLargestPossibleRegion());
  this->SegmentMask->Allocate();

  // Setup the image to store the node ids
  this->NodeImage = NodeImageType::New();
  this->NodeImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->NodeImage->Allocate();

  // Default paramters
  this->Lambda = 0.01;
  this->NumberOfHistogramBins = 10; // This value is never used - it is set from the slider

  // Initializations
  this->ForegroundHistogram = NULL;
  this->BackgroundHistogram = NULL;

  this->ForegroundSample = SampleType::New();
  this->BackgroundSample = SampleType::New();

  this->ForegroundHistogramFilter = SampleToHistogramFilterType::New();
  this->BackgroundHistogramFilter = SampleToHistogramFilterType::New();

  this->RGBWeight = 0.5; // This value is never used - it is set from the slider

}

void ImageGraphCut::CutGraph()
{
  std::cout << "RGBWeight: " << RGBWeight << std::endl;
  
  // Compute max-flow
  this->Graph->maxflow();

  // Setup the values of the output (mask) image
  //GrayscalePixelType sinkPixel;
  //sinkPixel[0] = 0;
  Mask::PixelType sinkPixel = 0;

  //GrayscalePixelType sourcePixel;
  //sourcePixel[0] = 255;
  Mask::PixelType sourcePixel = 255;

  // Iterate over the node image, querying the Kolmorogov graph object for the association of each pixel and storing them as the output mask
  itk::ImageRegionConstIterator<NodeImageType> nodeImageIterator(this->NodeImage,
                                                                 this->NodeImage->GetLargestPossibleRegion());
  nodeImageIterator.GoToBegin();

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

void ImageGraphCut::PerformSegmentation()
{
  // This function performs some initializations and then creates and cuts the graph

  // Ensure at least one pixel has been specified for both the foreground and background
  if((this->Sources.size() <= 0) || (this->Sinks.size() <= 0))
    {
    std::cerr << "At least one source (foreground) pixel and one sink (background) pixel must be specified!" << std::endl;
    std::cerr << "Currently there are " << this->Sources.size() << " and " << this->Sinks.size() << " sinks." << std::endl;
    return;
    }

  // Blank the NodeImage
  itk::ImageRegionIterator<NodeImageType> nodeImageIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  nodeImageIterator.GoToBegin();

  while(!nodeImageIterator.IsAtEnd())
    {
    nodeImageIterator.Set(NULL);
    ++nodeImageIterator;
    }

  // Blank the output image
  //itk::ImageRegionIterator<GrayscaleImageType> segmentMaskImageIterator(this->SegmentMask,
  //                                             this->SegmentMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<Mask> segmentMaskImageIterator(this->SegmentMask,
                                                                   this->SegmentMask->GetLargestPossibleRegion());
  segmentMaskImageIterator.GoToBegin();

  Mask::PixelType empty = 0;
  //empty[0] = 0;

  while(!segmentMaskImageIterator.IsAtEnd())
    {
    segmentMaskImageIterator.Set(empty);
    ++segmentMaskImageIterator;
    }

  this->CreateGraph();
  this->CutGraph();
}

void ImageGraphCut::CreateSamples()
{
  // This function creates ITK samples from the scribbled pixels and then computes the foreground and background histograms

  // We want the histogram bins to take values from 0 to 255 in all dimensions
  HistogramType::MeasurementVectorType binMinimum(this->Image->GetNumberOfComponentsPerPixel());
  HistogramType::MeasurementVectorType binMaximum(this->Image->GetNumberOfComponentsPerPixel());
  for(unsigned int i = 0; i < this->Image->GetNumberOfComponentsPerPixel(); i++)
    {
    binMinimum[i] = 0;
    binMaximum[i] = 255;
    }

  // Setup the histogram size
  std::cout << "Image components per pixel: " << this->Image->GetNumberOfComponentsPerPixel() << std::endl;
  SampleToHistogramFilterType::HistogramSizeType histogramSize(this->Image->GetNumberOfComponentsPerPixel());
  histogramSize.Fill(this->NumberOfHistogramBins);

  // Create foreground samples and histogram
  this->ForegroundSample->Clear();
  this->ForegroundSample->SetMeasurementVectorSize(this->Image->GetNumberOfComponentsPerPixel());
  //std::cout << "Measurement vector size: " << this->ForegroundSample->GetMeasurementVectorSize() << std::endl;
  //std::cout << "Pixel size: " << this->Image->GetPixel(this->Sources[0]).GetNumberOfElements() << std::endl;
  
  for(unsigned int i = 0; i < this->Sources.size(); i++)
    {
    this->ForegroundSample->PushBack(this->Image->GetPixel(this->Sources[i]));
    }

  this->ForegroundHistogramFilter->SetHistogramSize(histogramSize);
  this->ForegroundHistogramFilter->SetHistogramBinMinimum(binMinimum);
  this->ForegroundHistogramFilter->SetHistogramBinMaximum(binMaximum);
  this->ForegroundHistogramFilter->SetAutoMinimumMaximum(false);
  this->ForegroundHistogramFilter->SetInput(this->ForegroundSample);
  this->ForegroundHistogramFilter->Modified();
  this->ForegroundHistogramFilter->Update();

  this->ForegroundHistogram = this->ForegroundHistogramFilter->GetOutput();

  // Create background samples and histogram
  this->BackgroundSample->Clear();
  this->BackgroundSample->SetMeasurementVectorSize(this->Image->GetNumberOfComponentsPerPixel());
  for(unsigned int i = 0; i < this->Sinks.size(); i++)
    {
    this->BackgroundSample->PushBack(this->Image->GetPixel(this->Sinks[i]));
    }

  this->BackgroundHistogramFilter->SetHistogramSize(histogramSize);
  this->BackgroundHistogramFilter->SetHistogramBinMinimum(binMinimum);
  this->BackgroundHistogramFilter->SetHistogramBinMaximum(binMaximum);
  this->BackgroundHistogramFilter->SetAutoMinimumMaximum(false);
  this->BackgroundHistogramFilter->SetInput(this->BackgroundSample);
  this->BackgroundHistogramFilter->Modified();
  this->BackgroundHistogramFilter->Update();

  this->BackgroundHistogram = BackgroundHistogramFilter->GetOutput();

}

void ImageGraphCut::CreateGraph()
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

  // Estimate the "camera noise"
  double sigma = this->ComputeNoise();

  ////////// Create n-edges and set n-edge weights (links between image nodes) //////////

  // We are only using a 4-connected structure, so the kernel (iteration neighborhood) must only be 3x3 (specified by a radius of 1)
  itk::Size<2> radius;
  radius.Fill(1);

  typedef itk::ShapedNeighborhoodIterator<ImageType> IteratorType;

  // Traverse the image adding an edge between the current pixel and the pixel below it and the current pixel and the pixel to the right of it.
  // This prevents duplicate edges (i.e. we cannot add an edge to all 4-connected neighbors of every pixel or almost every edge would be duplicated.
  std::vector<IteratorType::OffsetType> neighbors;
  IteratorType::OffsetType bottom = {{0,1}};
  neighbors.push_back(bottom);
  IteratorType::OffsetType right = {{1,0}};
  neighbors.push_back(right);

  IteratorType::OffsetType center = {{0,0}};

  IteratorType iterator(radius, this->Image, this->Image->GetLargestPossibleRegion());
  iterator.ClearActiveList();
  iterator.ActivateOffset(bottom);
  iterator.ActivateOffset(right);
  iterator.ActivateOffset(center);

  for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
    PixelType centerPixel = iterator.GetPixel(center);

    for(unsigned int i = 0; i < neighbors.size(); i++)
      {
      bool valid;
      iterator.GetPixel(neighbors[i], valid);

      // If the current neighbor is outside the image, skip it
      if(!valid)
        {
        continue;
        }
      PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

      // Compute the Euclidean distance between the pixel intensities
      float pixelDifference = PixelDifference(centerPixel, neighborPixel);

      // Compute the edge weight
      float weight = exp(-pow(pixelDifference,2)/(2.0*sigma*sigma));
      assert(weight >= 0);

      // Add the edge to the graph
      void* node1 = this->NodeImage->GetPixel(iterator.GetIndex(center));
      void* node2 = this->NodeImage->GetPixel(iterator.GetIndex(neighbors[i]));
      this->Graph->add_edge(node1, node2, weight, weight);
      }
    }

  ////////// Add t-edges and set t-edge weights (links from image nodes to virtual background and virtual foreground node) //////////

  // Compute the histograms of the selected foreground and background pixels
  CreateSamples();

  itk::ImageRegionIterator<ImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<NodeImageType> nodeIterator(this->NodeImage, this->NodeImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  nodeIterator.GoToBegin();

  // Since the t-weight function takes the log of the histogram value,
  // we must handle bins with frequency = 0 specially (because log(0) = -inf)
  // For empty histogram bins we use tinyValue instead of 0.
  float tinyValue = 1e-10;

  while(!imageIterator.IsAtEnd())
    {
    PixelType pixel = imageIterator.Get();
    //std::cout << "Pixels have size: " << pixel.Size() << std::endl;

    HistogramType::MeasurementVectorType measurementVector(pixel.Size());
    for(unsigned int i = 0; i < pixel.Size(); i++)
      {
      measurementVector[i] = pixel[i];
      }

    HistogramType::IndexType backgroundIndex;
    this->BackgroundHistogram->GetIndex(measurementVector, backgroundIndex);
    float sinkHistogramValue = this->BackgroundHistogram->GetFrequency(backgroundIndex);

    HistogramType::IndexType foregroundIndex;
    this->ForegroundHistogram->GetIndex(measurementVector, foregroundIndex);
    float sourceHistogramValue = this->ForegroundHistogram->GetFrequency(foregroundIndex);

    // Conver the histogram value/frequency to make it as if it came from a normalized histogram
    sinkHistogramValue /= this->BackgroundHistogram->GetTotalFrequency();
    sourceHistogramValue /= this->ForegroundHistogram->GetTotalFrequency();

    if(sinkHistogramValue <= 0)
      {
      sinkHistogramValue = tinyValue;
      }
    if(sourceHistogramValue <= 0)
      {
      sourceHistogramValue = tinyValue;
      }

    // Add the edge to the graph and set its weight
    this->Graph->add_tweights(nodeIterator.Get(),
                              -this->Lambda*log(sinkHistogramValue),
                              -this->Lambda*log(sourceHistogramValue)); // log() is the natural log
    ++imageIterator;
    ++nodeIterator;
    }

  // Set very high source weights for the pixels which were selected as foreground by the user
  for(unsigned int i = 0; i < this->Sources.size(); i++)
    {
    this->Graph->add_tweights(this->NodeImage->GetPixel(this->Sources[i]),this->Lambda * std::numeric_limits<float>::max(),0);
    }

  // Set very high sink weights for the pixels which were selected as background by the user
  for(unsigned int i = 0; i < this->Sinks.size(); i++)
    {
    this->Graph->add_tweights(this->NodeImage->GetPixel(this->Sinks[i]),0,this->Lambda * std::numeric_limits<float>::max());
    }
}

float ImageGraphCut::PixelDifference(const PixelType& a, const PixelType& b)
{
  // Compute the Euclidean distance between N dimensional pixels
  float difference = 0;

  if(this->Image->GetNumberOfComponentsPerPixel() > 3)
    {
    for(unsigned int i = 0; i < 3; i++)
      {
      difference += (this->RGBWeight / 3.) * pow(a[i] - b[i],2);
      }
    for(unsigned int i = 3; i < this->Image->GetNumberOfComponentsPerPixel(); i++)
      {
      difference += (1 - this->RGBWeight) / (this->Image->GetNumberOfComponentsPerPixel() - 3.) * pow(a[i] - b[i],2);
      }
    }
  else // image is RGB or less (grayscale)
    {
    for(unsigned int i = 0; i < this->Image->GetNumberOfComponentsPerPixel(); i++)
      {
      difference += pow(a[i] - b[i],2);
      }
    }
  return sqrt(difference);
}

double ImageGraphCut::ComputeNoise()
{
  // Compute an estimate of the "camera noise". This is used in the N-weight function.

  // Since we use a 4-connected neighborhood, the kernel must be 3x3 (a rectangular radius of 1 creates a kernel side length of 3)
  itk::Size<2> radius;
  radius[0] = 1;
  radius[1] = 1;

  typedef itk::ShapedNeighborhoodIterator<ImageType> IteratorType;

  std::vector<IteratorType::OffsetType> neighbors;
  IteratorType::OffsetType bottom = {{0,1}};
  neighbors.push_back(bottom);
  IteratorType::OffsetType right = {{1,0}};
  neighbors.push_back(right);

  IteratorType::OffsetType center = {{0,0}};

  IteratorType iterator(radius, this->Image, this->Image->GetLargestPossibleRegion());
  iterator.ClearActiveList();
  iterator.ActivateOffset(bottom);
  iterator.ActivateOffset(right);
  iterator.ActivateOffset(center);

  double sigma = 0.0;
  int numberOfEdges = 0;

  // Traverse the image collecting the differences between neighboring pixel intensities
  for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
    PixelType centerPixel = iterator.GetPixel(center);

    for(unsigned int i = 0; i < neighbors.size(); i++)
      {
      bool valid;
      iterator.GetPixel(neighbors[i], valid);
      if(!valid)
        {
        continue;
        }
      PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

      float colorDifference = PixelDifference(centerPixel, neighborPixel);
      sigma += colorDifference;
      numberOfEdges++;
      }

    }

  // Normalize
  sigma /= static_cast<double>(numberOfEdges);

  return sigma;
}

ImageGraphCut::IndexContainer ImageGraphCut::GetSources()
{
  return this->Sources;
}

void ImageGraphCut::SetLambda(const float lambda)
{
  this->Lambda = lambda;
}

void ImageGraphCut::SetNumberOfHistogramBins(int bins)
{
  this->NumberOfHistogramBins = bins;
}

Mask* ImageGraphCut::GetSegmentMask()
{
  return this->SegmentMask;
}

ImageGraphCut::IndexContainer ImageGraphCut::GetSinks()
{
  return this->Sinks;
}

bool ImageGraphCut::IsNaN(const double a)
{
  return a != a;
}

void ImageGraphCut::SetSources(vtkPolyData* const sources)
{
  // Convert the vtkPolyData produced by the vtkImageTracerWidget to a list of pixel indices

  this->Sources.clear();

  for(vtkIdType i = 0; i < sources->GetNumberOfPoints(); i++)
    {
    itk::Index<2> index;
    double p[3];
    sources->GetPoint(i,p);
    /*
    index[0] = round(p[0]);
    index[1] = round(p[1]);
    */
    index[0] = vtkMath::Round(p[0]);
    index[1] = vtkMath::Round(p[1]);

    this->Sources.push_back(index);
    }

}

void ImageGraphCut::SetSinks(vtkPolyData* const sinks)
{
  // Convert the vtkPolyData produced by the vtkImageTracerWidget to a list of pixel indices

  this->Sinks.clear();

  for(vtkIdType i = 0; i < sinks->GetNumberOfPoints(); i++)
    {
    itk::Index<2> index;
    double p[3];
    sinks->GetPoint(i,p);
    /*
    index[0] = round(p[0]);
    index[1] = round(p[1]);
    */
    index[0] = vtkMath::Round(p[0]);
    index[1] = vtkMath::Round(p[1]);

    this->Sinks.push_back(index);
    }

}

void ImageGraphCut::SetSources(const IndexContainer& sources)
{
  this->Sources = sources;
}

void ImageGraphCut::SetSinks(const IndexContainer& sinks)
{
  this->Sinks = sinks;
}

ImageType* ImageGraphCut::GetImage()
{
  return this->Image;
}
