#include "ImageGraphCut.h"
#include "Helpers.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkVectorImage.h"

int main(int argc, char*argv[])
{
  typedef itk::VectorImage<float, 2> ImageType;
  
  if(argc != 5)
    {
    std::cerr << "Required: image foregroundMask backgroundMask output" << std::endl;
    return EXIT_FAILURE;
    }

  std::string imageFilename = argv[1];
  std::string foregroundFilename = argv[2]; // This image should have white pixels indicating foreground pixels and be black elsewhere.
  std::string backgroundFilename = argv[3]; // This image should have white pixels indicating background pixels and be black elsewhere.
  std::string outputFilename = argv[4]; // Foreground/background segment mask

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(imageFilename);
  reader->Update();

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> UnsignedCharReaderType;
  UnsignedCharReaderType::Pointer foregroundReader = UnsignedCharReaderType::New();
  foregroundReader->SetFileName(foregroundFilename);
  foregroundReader->Update();

  UnsignedCharReaderType::Pointer backgroundReader = UnsignedCharReaderType::New();
  backgroundReader->SetFileName(backgroundFilename);
  backgroundReader->Update();
  
  ImageGraphCut GraphCut;
  GraphCut.SetImage(reader->GetOutput());
  GraphCut.SetNumberOfHistogramBins(20);
  GraphCut.SetLambda(.01);
  std::vector<itk::Index<2> > foregroundPixels = Helpers::GetNonZeroPixels(foregroundReader->GetOutput());
  std::vector<itk::Index<2> > backgroundPixels = Helpers::GetNonZeroPixels(backgroundReader->GetOutput());
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
