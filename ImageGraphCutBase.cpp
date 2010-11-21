#include "ImageGraphCut.h"

#include "itkImageRegionIterator.h"
#include "itkShapedNeighborhoodIterator.h"

#include <cmath>

#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

ImageGraphCutBase::ImageGraphCutBase()
{
  this->SegmentMask = MaskImageType::New();
  this->NodeImage = NodeImageType::New();

  this->Lambda = 0.01;
  this->NumberOfHistogramBins = 10;
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