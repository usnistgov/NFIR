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
 *
 * @param srcSampleRate source image ppi to be upsampled
 * @param tgtSampleRate target image ppi of resulting upsample
 */
Upsample::Upsample( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _resizeFactor = (float)_tgtSampleRate / (float)_srcSampleRate;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Upsample() {}.
// *********

/** Wrapper for the OpenCV `resize` function.  Uses resize factor and
 * interpolation method.
 *
 * @param srcImg to be resized by the amount of the `resizeFactor`
 * @return target resized image
 */
cv::Mat Upsample::resize( cv::Mat srcImg )
{
  cv::Mat resampledImg;
  cv::resize( srcImg, resampledImg, cv::Size(),
              _resizeFactor, _resizeFactor, _interpolationMethod );
  // resampledImg.release();  // Force test NFIR::Miscue
  return resampledImg;
}


/** Wrapper for the OpenCV `resize` function.
 *   NOT TO BE CALLED; overridden to satisfy linker.
 * @param srcImg to be resized by the amount of the __resizeFactor__
 * @param filterMask that is multiplied with the freq domain image
 * @param pads amount to crop the space domain image
 *
 * @return target resized image
 */
cv::Mat Upsample::resize( cv::Mat srcImg, NFIR::FilterMask* filterMask, Padding& pads )
{
  filterMask->~FilterMask();
  pads.reset();
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


/**
 * @param im interpolation method `bicubic` or `bilinear`
 *
 * @throw invalid interpolation method
*/
void Upsample::set_interpolationMethod( const std::string im )
{
  _configRecap = "Interpolation method specified by user (per config).";
  if( im == "bicubic" )
  {
    _interpolationMethod = cv::INTER_CUBIC;
  }
  else if( im == "bilinear" )
  {
    _interpolationMethod = cv::INTER_LINEAR;
  }
  else if( im == "" )
  {
    _configRecap = "Using recommended interpolation method.";
    _interpolationMethod = determineInterpolation( get_srcSampleRate() );
  }
  else
    throw NFIR::Miscue( "NFIR lib: invalid interpolation method: " + im );
}


std::vector<std::string> Upsample::to_s(void) const
{
  std::vector<std::string> v;
  v.push_back("UPSAMPLE configuration:");
  v.push_back("  source sample rate:  " + std::to_string(get_srcSampleRate()) );
  v.push_back("  target sample rate:  " + std::to_string(get_tgtSampleRate()) );
  v.push_back("  resize factor:       " + std::to_string(get_resizeFactor()) );
  v.push_back("Interpolation: " + _configRecap );
  v.push_back("  interpolation method:  " + std::to_string(get_interpolationMethod())
            + "  (1=bilinear, 2=bicubic)\n" );
  return v;
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
