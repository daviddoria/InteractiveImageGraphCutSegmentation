#include "itkImageRegionConstIteratorWithIndex.h"

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

} // end namespace