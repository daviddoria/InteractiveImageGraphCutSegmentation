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

// ITK
#include "itkImageRegionIterator.h"
#include "itkShapedNeighborhoodIterator.h"

// STL
#include <cmath>

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

ImageGraphCutBase::ImageGraphCutBase()
{
  // Initializations
  this->SegmentMask = MaskImageType::New();
  this->NodeImage = NodeImageType::New();

  // Default values
  this->Lambda = 0.01;
  this->NumberOfHistogramBins = 10;
  this->PixelDimensionality = 0;
  this->RGBWeight = 0.5;
  //this->RGBWeight = 1.0;
  //this->RGBWeight = 0;
}

unsigned int ImageGraphCutBase::GetPixelDimensionality()
{
  return this->PixelDimensionality;
}

void ImageGraphCutBase::SetRGBWeight(float weight)
{
  this->RGBWeight = weight;
}

std::vector<itk::Index<2> > ImageGraphCutBase::GetSources()
{
  return this->Sources;
}

void ImageGraphCutBase::SetLambda(float lambda)
{
  this->Lambda = lambda;
}

void ImageGraphCutBase::SetNumberOfHistogramBins(int bins)
{
  this->NumberOfHistogramBins = bins;
}

MaskImageType::Pointer ImageGraphCutBase::GetSegmentMask()
{
  return this->SegmentMask;
}

std::vector<itk::Index<2> > ImageGraphCutBase::GetSinks()
{
  return this->Sinks;
}

bool ImageGraphCutBase::IsNaN(const double a)
{
  return a != a;
}


void ImageGraphCutBase::SetSources(vtkPolyData* sources)
{
  // Convert the vtkPolyData produced by the vtkImageTracerWidget to a list of pixel indices

  this->Sources.clear();

  for(vtkIdType i = 0; i < sources->GetNumberOfPoints(); i++)
    {
    itk::Index<2> index;
    double p[3];
    sources->GetPoint(i,p);
    index[0] = round(p[0]);
    index[1] = round(p[1]);

    this->Sources.push_back(index);
    }

}

void ImageGraphCutBase::SetSinks(vtkPolyData* sinks)
{
  // Convert the vtkPolyData produced by the vtkImageTracerWidget to a list of pixel indices

  this->Sinks.clear();

  for(vtkIdType i = 0; i < sinks->GetNumberOfPoints(); i++)
    {
    itk::Index<2> index;
    double p[3];
    sinks->GetPoint(i,p);
    index[0] = round(p[0]);
    index[1] = round(p[1]);

    this->Sinks.push_back(index);
    }

}

void ImageGraphCutBase::SetSources(std::vector<itk::Index<2> > sources)
{
  this->Sources = sources;
}

void ImageGraphCutBase::SetSinks(std::vector<itk::Index<2> > sinks)
{
  this->Sinks = sinks;
}