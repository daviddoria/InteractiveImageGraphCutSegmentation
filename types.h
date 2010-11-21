#ifndef TYPES_H
#define TYPES_H

#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkSampleToHistogramFilter.h"
#include "itkHistogram.h"
#include "itkListSample.h"

typedef itk::CovariantVector<float,3> ColorPixelType;
typedef itk::CovariantVector<float,1> GrayscalePixelType;

typedef itk::Image<ColorPixelType, 2> ColorImageType;
typedef itk::Image<GrayscalePixelType, 2> GrayscaleImageType;

typedef itk::Image<GrayscalePixelType, 2> MaskImageType;
//typedef itk::Image<bool, 2> MaskImageType;

/*
typedef itk::Image<ColorPixelType, 2> ColorImageType;

typedef unsigned char GrayscalePixelType;
typedef itk::Image<GrayscalePixelType, 2> GrayscaleImageType;
*/

typedef itk::Image<void*, 2> NodeImageType;

// Color Histograms


typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 > HistogramType;

//typedef itk::Statistics::SampleToHistogramFilter<ColorSampleType, ColorHistogramType> ColorSampleToHistogramFilterType;

/*
// Grayscale Histograms
typedef itk::Statistics::ListSample<itk::CovariantVector<unsigned char,1> > GrayscaleSampleType ;

typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 > GrayscaleHistogramType;

typedef itk::Statistics::SampleToHistogramFilter<GrayscaleSampleType, GrayscaleHistogramType> GrayscaleSampleToHistogramFilterType;
*/

#endif