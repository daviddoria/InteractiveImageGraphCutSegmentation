#ifndef HELPERS_H
#define HELPERS_H

#include "itkImage.h"
#include "itkIndex.h"

#include "Types.h"

// Mark each pixel at the specified 'indices' as a non-zero pixel in 'image'
void IndicesToBinaryImage(std::vector<itk::Index<2> > indices, UnsignedCharScalarImageType::Pointer image);

  
#endif