#ifndef HELPERS_H
#define HELPERS_H

// ITK
#include "itkImage.h"
#include "itkIndex.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
class vtkPolyData;
class vtkPoints;

// Custom
#include "Types.h"

namespace Helpers
{

void MaskImage(vtkSmartPointer<vtkImageData> VTKImage, vtkSmartPointer<vtkImageData> VTKSegmentMask, vtkSmartPointer<vtkImageData> VTKMaskedImage);


std::vector<itk::Index<2> > GetNonZeroPixels(const MaskImageType* const image);

std::vector<itk::Index<2> > PolyDataToPixelList(vtkPolyData* const polydata);

void PixelListToPoints(const std::vector<itk::Index<2> >& pixels, vtkPoints* const points);

// Mark each pixel at the specified 'indices' as a non-zero pixel in 'image'
void IndicesToBinaryImage(std::vector<itk::Index<2> > indices, UnsignedCharScalarImageType::Pointer image);

// Invert binary image
void InvertBinaryImage(UnsignedCharScalarImageType::Pointer image, UnsignedCharScalarImageType::Pointer inverted);

void ITKImagetoVTKImage(ImageType::Pointer image, vtkImageData* outputImage); // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage
void ITKImagetoVTKRGBImage(ImageType::Pointer image, vtkImageData* outputImage);
void ITKImagetoVTKMagnitudeImage(ImageType::Pointer image, vtkImageData* outputImage);

void ITKScalarImagetoVTKImage(MaskImageType::Pointer image, vtkImageData* outputImage);

/** Write 'image' to 'fileName'.*/
template<typename TImage>
void WriteImage(const TImage* const image, const std::string& fileName);

template<typename TImage, typename TMask>
void MaskImage(const TImage* const image, const TMask* const mask, TImage* const output);

/** Deep copy an image. */
template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output);

/** Deep copy a vector image.
 * Note: specialization declarations must appear in the header or the compiler does
 * not know about their definition in the .cpp file!
 */
template<>
void DeepCopy<itk::VectorImage<float, 2> >(const itk::VectorImage<float, 2>* const input, itk::VectorImage<float, 2>* const output);


template<typename TImage>
void DeepCopyInRegion(const TImage* const input, const itk::ImageRegion<2>& region, TImage* const output);


}

#include "Helpers.hpp"

#endif