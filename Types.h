#ifndef TYPES_H
#define TYPES_H

#include "itkImage.h"
#include "itkVectorImage.h"

// All images are stored internally as float pixels
typedef itk::VectorImage<float,2> ImageType;
typedef ImageType::PixelType PixelType;
//typedef typename ImageType::PixelType PixelType;

//typedef itk::VariableLengthVector<float> PixelType;

typedef itk::Image<float, 2> FloatScalarImageType;

// For writing images, we need to first convert to actual unsigned char images
typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
//typedef UnsignedCharScalarImageType MaskImageType;

#endif