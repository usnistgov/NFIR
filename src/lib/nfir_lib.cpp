/*******************************************************************************
License:
This software was developed at the National Institute of Standards and
Technology (NIST) by employees of the Federal Government in the course
of their official duties. Pursuant to title 17 Section 105 of the
United States Code, this software is not subject to copyright protection
and is in the public domain. NIST assumes no responsibility  whatsoever for
its use by other parties, and makes no guarantees, expressed or implied,
about its quality, reliability, or any other characteristic.

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain. Therefore,
it is permissible to distribute this software as a free download from the
internet.

Disclaimer:
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.

*******************************************************************************/
#include "exceptions.h"
#include "filter_mask_gaussian.h"
#include "filter_mask_ideal.h"
#include "nfir_lib.h"
#include "resample_down.h"
#include "resample_up.h"


/** Library private methods */
static cv::Mat padImage( cv::Mat, Padding& );
static void validateSampleRate( int, int );

namespace NFIR {

/**
This method is the interface into the NIST Fingerprint Image Resampler library.
The caller only need use this method to perform the entire, resample process
on a single image.

It instantiates the up or downsample object based on the source and target
sample rates.  The up- or down-sample "resize factor" = (target rate)/(source rate).

For upsample, perform the resize and return.

For downsample, pad the source image, build the lowpass filter, Fourier
transform the padded image and matrix multiply with the filter (or mask).
Inverse-Fourier transform the product, strip the image padding, perform
final resize.


@param srcImage point to source image in memory
@param tgtImage point to target image in memory
@param srcSampleRate
@param tgtSampleRate
@param interpolationMethod [ bilinear | default to bicubic ]
@param filterShape [ ideal | default to gaussian ]

@throw NFIR::Miscue for invalid sample rate(s), interpolation method, downsample filter shape, or cannot resize image
*/
void resample( cv::Mat &srcImage, cv::Mat &tgtImage,
               int srcSampleRate, int tgtSampleRate,
               std::string interpolationMethod,
               std::string filterShape )
{
  int errCode{0};
  validateSampleRate( srcSampleRate, tgtSampleRate );

  // To be sure, clear the target.
  tgtImage.release();

  /** Polymorphic, single instance either Upsample or Downsample */
  Resample *resampler;
  /** Polymorphic, single instance Gaussian or Ideal */
  FilterMask *currentFilter;

  cv::Mat padded;
  Padding actualPadSize;


  // Based on UP or DOWN sample, instantiate the proper object.
  if( tgtSampleRate > srcSampleRate )
  {
    resampler = new Upsample( srcSampleRate, tgtSampleRate );
    errCode = resampler->set_interpolationMethod( interpolationMethod );
  }
  else
  {
    resampler = new Downsample( srcSampleRate, tgtSampleRate );
    errCode = resampler->set_interpolationMethodAndFilterShape( interpolationMethod, filterShape );
  }
  if( errCode == -1 ) {
    throw NFIR::Miscue( "NFIR lib: invalid parameter interpolation method: '" + interpolationMethod + "'" );
  }
  else if( errCode == -2 ) {
    throw NFIR::Miscue( "NFIR lib: invalid parameter filter shape: '" + filterShape + "'");
  }

  if( tgtSampleRate > srcSampleRate )
  {
    tgtImage = resampler->resize( srcImage );
    if( tgtImage.empty() ) {
      throw NFIR::Miscue( "NFIR lib: Upsample failed resize()" );
    }
    return;
  }

  // Not an UPSAMPLE, so start the DOWNSAMPLE process.
  padded = padImage( srcImage, actualPadSize );

  // Build the filter/mask for freq domain mulSpectums.
  // The filter/mask is same dimension (WxH) as the padded, source image.
  try
  {
    if( resampler->get_filterShape() == "gaussian" )
    {
      currentFilter = new Gaussian( srcSampleRate, tgtSampleRate );
      currentFilter->build( padded.size() );
    }
    else if( resampler->get_filterShape() == "ideal" )
    {
      currentFilter = new Ideal( srcSampleRate, tgtSampleRate );
      currentFilter->build( padded.size() );
    }
    else
    {
      // Although filter shape has already been verfied, the compiler throws an error
      // that currentFilter may not have been initialized.
      throw NFIR::Miscue( "NFIR lib: invalid parameter filter shape: '" + resampler->get_filterShape() + "'");
    }

    // Now that the padded, source image and "current" filter mask are available...
    tgtImage = resampler->resize( padded, currentFilter, actualPadSize );
  }
  catch( const cv::Exception& ex ) {
    std::string err{"NFIR lib: Downsample failed resize(): "};
    err.append( ex.what() );
    throw NFIR::Miscue( err );
  }


  // Clean up
  delete resampler;
  resampler = nullptr;
  delete currentFilter;
  currentFilter = nullptr;

  return;
}


/**
*/
std::string printVersion()
{
  std::string s{ "NFIR (NIST Fingerprint Image Resampler) version: " };
  std::string v{ NFIR_VERSION };
  s.append(v);
  s.append( "\nOpenCV version: ");
  s.append(CV_VERSION);
  return s;
}

}   // End namespace



/** Utilize the OpenCV optimal padding function.  If either (or both) of the optimal
rows or columns are odd, one row or column is added to the padding.  This must be
done to ensure that the ideal filter/mask rightmost column and bottommost row
contain all zeros.

@param image to pad
@param actual OUT padding values

@return the padded image
*/
cv::Mat padImage( cv::Mat image, Padding &actual )
{
  cv::Mat padded;
  int optimalRows = cv::getOptimalDFTSize( image.rows );
  if (optimalRows % 2)  // odd
    optimalRows++;
  int optimalCols = cv::getOptimalDFTSize( image.cols );
  if (optimalCols % 2)  // odd
    optimalCols++;
  int pad_rows = optimalRows - image.rows;
  int pad_cols = optimalCols - image.cols;
  cv::copyMakeBorder( image, padded, 0, pad_rows, 0, pad_cols,
                      cv::BORDER_CONSTANT, cv::Scalar::all(255) );
  actual.top = 0;
  actual.bottom = pad_rows;
  actual.left = 0;
  actual.right = pad_cols;

  return padded;
}

/**
Validate the sample rates not handled by CLI11.

@throw NFIR::Miscue sample rates cannot be equal or negative
*/
void validateSampleRate( int srcSampleRate, int tgtSampleRate )
{
  if( tgtSampleRate == srcSampleRate )
  {
    throw NFIR::Miscue( "NFIR lib: src and target sample rates cannot be equal" );
  }

  if( (srcSampleRate <= 0 ) || ( tgtSampleRate <= 0 ) )
  {
    throw NFIR::Miscue( "NFIR lib: src and/or target sample rate cannot be negative" );
  }
}
