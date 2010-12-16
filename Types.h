#ifndef TYPES_H
#define TYPES_H

#include "itkImage.h"
#include "itkCovariantVector.h"

// All images are stored internally as float pixels
typedef itk::CovariantVector<float,3> ColorPixelType;
typedef itk::CovariantVector<float,1> GrayscalePixelType;
typedef itk::CovariantVector<float,5> RGBDIPixelType;

typedef itk::Image<ColorPixelType, 2> ColorImageType;
typedef itk::Image<GrayscalePixelType, 2> GrayscaleImageType;
typedef itk::Image<RGBDIPixelType, 2> RGBDIImageType;

//typedef itk::Image<GrayscalePixelType, 2> MaskImageType;

// For writing images, we need to first convert to actual unsigned char images
typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
typedef UnsignedCharScalarImageType MaskImageType;

#endif