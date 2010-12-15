#include "ImageGraphCut.h"

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIteratorWithIndex.h"

typedef itk::CovariantVector<float,5> RGBDIPixelType;
typedef itk::Image<RGBDIPixelType, 2> RGBDIImageType;

typedef itk::Image<unsigned char, 2> UnsignedCharImageType;

std::vector<itk::Index<2> > GetMaskedPixels(UnsignedCharImageType::Pointer image);

int main(int argc, char*argv[])
{
  std::string imageFilename = argv[1];
  std::string foregroundMaskFilename = argv[2];
  std::string backgroundMaskFilename = argv[3];

  typedef itk::ImageFileReader<RGBDIImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(imageFilename);
  reader->Update();

  typedef itk::ImageFileReader<UnsignedCharImageType> UnsignedCharReaderType;
  UnsignedCharReaderType::Pointer foregroundMaskReader = UnsignedCharReaderType::New();
  foregroundMaskReader->SetFileName(foregroundMaskFilename);
  foregroundMaskReader->Update();

  UnsignedCharReaderType::Pointer backgroundMaskReader = UnsignedCharReaderType::New();
  backgroundMaskReader->SetFileName(backgroundMaskFilename);
  backgroundMaskReader->Update();
  
  ImageGraphCut<RGBDIImageType> GraphCut(reader->GetOutput());
  GraphCut.SetNumberOfHistogramBins(20);
  GraphCut.SetLambda(.01);
  std::vector<itk::Index<2> > foregroundPixels = GetMaskedPixels(foregroundMaskReader->GetOutput());
  std::vector<itk::Index<2> > backgroundPixels = GetMaskedPixels(backgroundMaskReader->GetOutput());
  GraphCut.SetSources(foregroundPixels);
  GraphCut.SetSinks(backgroundPixels);

  //ProgressThread.start();
}


std::vector<itk::Index<2> > GetMaskedPixels(UnsignedCharImageType::Pointer image)
{
  std::vector<itk::Index<2> > pixels;
  
  itk::ImageRegionConstIteratorWithIndex<UnsignedCharImageType> imageIterator(image,image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() != 0)
      {
      pixels.push_back(imageIterator.GetIndex());
      }

    ++imageIterator;
    }

  return pixels;
}