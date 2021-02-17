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
#include "resample_up.h"

#include <iostream>

static InterpolationMethod determineInterpolation(int);

namespace NFIR {

// Default constructor.
Upsample::Upsample()
{
  Init();
}
// Copy constructor.
Upsample::Upsample( const Upsample& aCopy ) : Resample::Resample( aCopy )
{
  Copy( aCopy );
}

/** Full constructor.  Calculates the image resize factor.

@param srcSampleRate source image ppi to be upsampled
@param tgtSampleRate target image ppi of resulting upsample
*/
Upsample::Upsample( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _resizeFactor = (float)_tgtSampleRate / (float)_srcSampleRate;

  dirty = true;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Upsample() {}.
// *********

/** Wrapper for the OpenCV `resize` function.  Uses resize factor and
interpolation method.

@param srcImg to be resized by the amount of the `resizeFactor`

@return target resized image
*/
cv::Mat Upsample::resize( cv::Mat srcImg )
{
  cv::Mat tgtImg;
  cv::resize( srcImg, tgtImg, cv::Size(0, 0), _resizeFactor, _resizeFactor, _interpolationMethod );
  return tgtImg;
}


/** Wrapper for the OpenCV `resize` function.  NOT TO BE CALLED; overridden to satisfy linker.

@param srcImg to be resized by the amount of the __resizeFactor__
@param filterMask that is multiplied with the freq domain image
@param pads amount to crop the space domain image

@return target resized image
*/
cv::Mat Upsample::resize( cv::Mat srcImg, NFIR::FilterMask* filterMask, Padding& pads )
{
  filterMask->~FilterMask();
  pads.clear();
  return srcImg;
}


Upsample Upsample::Clone(void)
{
  Upsample c;
  c.Copy( *this );
  return c;
}

Upsample Upsample::operator=( const Upsample& aCopy )
{
  Copy( aCopy );
  return *this;
}


/** Implements the recommended interpolation method if this config param
is not set by user.

@param im interpolation method `bicubic` or `bilinear`

@return 0 upon success, -1 for invalid interpolation method
*/
int Upsample::set_interpolationMethod( const std::string im )
{
  if( im == "bicubic" )
  {
    _configRecap = "Interpolation method specified by user (per config).";
    _interpolationMethod = cv::INTER_CUBIC;
  }
  else if( im == "bilinear" )
  {
    _configRecap = "Interpolation method specified by user (per config).";
    _interpolationMethod = cv::INTER_LINEAR;
  }
  else if( im == "" )
  {
    _configRecap = "Using recommended interpolation method.";
    _interpolationMethod = determineInterpolation(get_srcSampleRate());
  }
  else
    return -1;
  return 0;
}


/** Print to console this instance configuration.
*/
void Upsample::to_s(void) const
{
  std::cout << "UPSAMPLE configuration:" << std::endl;
  std::cout << "  source sample rate:  " << get_srcSampleRate() << std::endl;
  std::cout << "  target sample rate:  " << get_tgtSampleRate() << std::endl;
  std::cout << "  resize factor:       " << get_resizeFactor() << std::endl;
  std::cout << std::endl;
  std::cout << "Interpolation config: " << _configRecap << std::endl;
  std::cout << "  interpolation method:  " << get_interpolationMethod()
            << "  (1=bilinear, 2=bicubic)" << std::endl;
}

}   // End namespace

/** Helper to set the default interpolation method.
*/
InterpolationMethod determineInterpolation( int rate )
{
  InterpolationMethod meth;
  switch ( rate )
  {
    case 300:
    case 400:
    default:
      meth = cv::INTER_CUBIC;
  }
  return meth;
}
