#include "itkImageFileWriter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkMaskImageFilter.h"
#include "itkVectorImage.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

namespace Helpers
{

#if 0
// Specializations (must go before the calls to these functions - prevents "specialization after instantiation" errors)
// Specializationso RGBDI images are displayed using RGB only
template <>
void ITKImagetoVTKImage<RGBDIImageType>(RGBDIImageType::Pointer image, vtkImageData* outputImage)
{
  // Setup and allocate the image data
  outputImage->SetNumberOfScalarComponents(3); // we are definitly making an RGB image
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the input image pixels to the output image
  itk::ImageRegionConstIteratorWithIndex<RGBDIImageType> imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    for(unsigned int component = 0; component < 3; component++) // we explicitly stop at 3
      {
      pixel[component] = static_cast<unsigned char>(imageIterator.Get()[component]);
      }

    ++imageIterator;
    }
}
#endif


template<typename TPixel>
void ExtractChannel(const itk::VectorImage<TPixel, 2>* const image, const unsigned int channel,
                    itk::Image<TPixel, 2>* const output)
{
  typedef itk::VectorImage<TPixel, 2> VectorImageType;
  typedef itk::Image<TPixel, 2> ScalarImageType;

  typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ScalarImageType > IndexSelectionType;
  typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(channel);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();

  DeepCopy(indexSelectionFilter->GetOutput(), output);
}

template<typename TImage>
void WriteImage(const TImage* const image, const std::string& filename)
{
  // This is a convenience function so that images can be written in 1 line instead of 4.
  typename itk::ImageFileWriter<TImage>::Pointer writer = itk::ImageFileWriter<TImage>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}

template<typename TImage, typename TMask>
void MaskImage(const TImage* const image, const TMask* const mask, TImage* const output)
{
  // Note: If you get a compiler error on this function complaining about NumericTraits in MaskImageFilter,
  // you will need a newer version of ITK. The ability to mask a VectorImage is new.

  // Mask the input image with the mask
  //typedef itk::MaskImageFilter< TImage, GrayscaleImageType > MaskFilterType;
  typedef itk::MaskImageFilter< ImageType, MaskImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();

  typedef itk::VariableLengthVector<double> VariableVectorType;
  VariableVectorType variableLengthVector;
  variableLengthVector.SetSize(image->GetNumberOfComponentsPerPixel());
  variableLengthVector.Fill(0);
  maskFilter->SetOutsideValue(variableLengthVector);

  maskFilter->SetInput1(image);
  maskFilter->SetInput2(mask);
  maskFilter->Update();

  Helpers::DeepCopy(maskFilter->GetOutput(), output);
}


/** Copy the input to the output*/
template<typename TImage>
void DeepCopy(const TImage* input, TImage* output)
{
  //std::cout << "DeepCopy()" << std::endl;
  if(output->GetLargestPossibleRegion() != input->GetLargestPossibleRegion())
    {
    output->SetRegions(input->GetLargestPossibleRegion());
    output->Allocate();
    }
  DeepCopyInRegion<TImage>(input, input->GetLargestPossibleRegion(), output);
}

template<typename TImage>
void DeepCopyInRegion(const TImage* input, const itk::ImageRegion<2>& region, TImage* output)
{
  // This function assumes that the size of input and output are the same.

  itk::ImageRegionConstIterator<TImage> inputIterator(input, region);
  itk::ImageRegionIterator<TImage> outputIterator(output, region);

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

} // end namespace