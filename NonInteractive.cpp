#include "ImageGraphCut.h"

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIteratorWithIndex.h"

std::vector<itk::Index<2> > GetMaskedPixels(UnsignedCharScalarImageType::Pointer image);

int main(int argc, char*argv[])
{
  if(argc != 5)
    {
    std::cerr << "Required: image foregroundMask backgroundMask output" << std::endl;
    return EXIT_FAILURE;
    }
    
  std::string imageFilename = argv[1];
  std::string foregroundMaskFilename = argv[2];
  std::string backgroundMaskFilename = argv[3];
  std::string outputFilename = argv[4]; // Foreground/background segment mask

  typedef itk::ImageFileReader<RGBDIImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(imageFilename);
  reader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> UnsignedCharReaderType;
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
  GraphCut.PerformSegmentation();

  UnsignedCharScalarImageType* result = GraphCut.GetSegmentMask();

  typedef  itk::ImageFileWriter< MaskImageType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(result);
  writer->Update();
}


std::vector<itk::Index<2> > GetMaskedPixels(UnsignedCharScalarImageType::Pointer image)
{
  std::vector<itk::Index<2> > pixels;
  
  itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> imageIterator(image,image->GetLargestPossibleRegion());

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