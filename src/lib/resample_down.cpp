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
#include "resample_down.h"

#include <iostream>

static cv::Mat applyFilterFreqDomain( cv::Mat complexI, cv::Mat mask );


namespace NFIR {

// Default constructor.
Downsample::Downsample()
{
  Init();
}
// Copy constructor.
Downsample::Downsample( const Downsample& aCopy ) : Resample::Resample( aCopy )
{
  Copy( aCopy );
}

/** Full constructor.  Calculates the image resize factor.

@param srcSampleRate source image ppi to be downsampled
@param tgtSampleRate target image ppi of resulting downsample
*/
Downsample::Downsample( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;
  _resizeFactor = (float)_tgtSampleRate / (float)_srcSampleRate;

  dirty = true;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Downsample() {}.
// *********


/** Includes image processing prior to the OpenCV `resize` function:

1. performs the forward fourier transform of the `srcImg`
2. factors-out the DC component (by dividing each freq by size of `srcImg`)
3. applies the filter/mask
4. takes the inverse transform
5. extracts image that is (polar) magnitude of inverse Fourier transform
6. crops space domain image
7. calls the OpenCV `resize` function.

The `resize` function uses the __resize factor__ and interpolation method.

@param srcImg to be resized by the amount of the __resizeFactor__
@param filterMask that is multiplied with the freq domain image
@param pads amount to crop the space domain image

@return final downsampled, resized image for write to disk
*/
cv::Mat Downsample::resize( cv::Mat srcImg, NFIR::FilterMask* filterMask, Padding& pads )
{
  cv::Mat resampledImg;
  try
  {
    cv::Mat fourierTransform, scaledFwdDFT;

    // ------ STEP #1) Forward Fourier transform.
    cv::Mat planes[] = { cv::Mat_<float>(srcImg), cv::Mat::zeros( srcImg.size(), CV_32F ) };
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);

    cv::dft( complexI, fourierTransform, cv::DFT_COMPLEX_OUTPUT );
    // ------ STEP #2) Scale by dividing by the DC component.
    cv::multiply( fourierTransform, (float)1/srcImg.total(), scaledFwdDFT );

    // ------ STEP #3) Apply filter/mask to image spectrum in freq domain.
    // Multiplication in freq domain where the (multiplicands/factors) spectrums
    // have DC term in center.  The filter/mask was constructed to have DC term
    // in center, therefore, shift the fourier transform signal.
    cv::Mat filteredSpectrum = applyFilterFreqDomain( scaledFwdDFT, filterMask->get_theMask() );

    // ------ STEP #4) Inverse Fourier transform.
    cv::Mat inverseTransform;
    cv::dft( filteredSpectrum, inverseTransform, cv::DFT_INVERSE );

    // ------ STEP #5) extract image that is (polar) magnitude of inverse Fourier transform.
    cv::Mat planes2[] = { cv::Mat::zeros( inverseTransform.size(), CV_32F ), cv::Mat::zeros( inverseTransform.size(), CV_32F ) };
    cv::split(inverseTransform, planes2);                 // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    cv::Mat finalImage;
    planes2[0].convertTo(finalImage, CV_8U);

    // ------ STEP #6) Crop padding.
    int startX=pads.left;  // since padding only right and bottom, this is 0.
    int startY=pads.top;   // since padding only right and bottom, this is 0.
    int cropWidth=srcImg.cols - pads.right;
    int cropHeight=srcImg.rows - pads.bottom;
    cv::Mat croppedImage( finalImage, cv::Rect(startX,startY,cropWidth,cropHeight) );

    // ------ STEP #7) Downsize to target ppi.
    cv::resize( croppedImage, resampledImg, cv::Size(0, 0), _resizeFactor, _resizeFactor, _interpolationMethod );
  }
  catch( const cv::Exception& ex ) {
    throw ex;
  }

  return resampledImg;
}


/** Wrapper for the OpenCV `resize` function.  Uses resize factor and
interpolation method.  NOT TO BE CALLED; overridden to satisfy linker.

@param srcImg to be resized by the amount of the `resizeFactor`

@return target resized image
*/
cv::Mat Downsample::resize( cv::Mat srcImg )
{
  return srcImg;
}



Downsample Downsample::Clone(void)
{
  Downsample c;
  c.Copy( *this );
  return c;
}

Downsample Downsample::operator=( const Downsample& aCopy )
{
  Copy( aCopy );
  return *this;
}


/** Print to console this instance configuration.
*/
void Downsample::to_s(void) const
{
  std::cout << "DOWNSAMPLE configuration:" << std::endl;
  std::cout << "  source sample rate:  " << get_srcSampleRate() << std::endl;
  std::cout << "  target sample rate:  " << get_tgtSampleRate() << std::endl;
  std::cout << "  resize factor:       " << get_resizeFactor() << std::endl;
  std::cout << std::endl;
  std::cout << "Filter/Interpolation config: " << _configRecap << std::endl;
  std::cout << "  filter/mask shape:     " << _filterShape << std::endl;
  std::cout << "  interpolation method:  " << get_interpolationMethod()
            << "  (1=bilinear, 2=bicubic)" << std::endl;
  std::cout << std::endl;
}


/** Implements the recommended filter shape and interpolation method if these
config params not set by user.  See class FilterMask::FilterShape.

   **IDEAL LPF, use interpolation:
      a.  600 to 500ppi - BICUBIC     <=== BEST, set by this method
      b. 1000 to 500ppi - BILINEAR    <=== BEST
      c. 1200 to 500ppi - BILINEAR    <=== MANUAL, must specify per config params
   **GAUSSIAN LPF, use interpolation:
      a.  600 to 500ppi - BICUBIC     <=== MANUAL
      b. 1000 to 500ppi - BICUBIC     <=== MANUAL
      c. 1200 to 500ppi - BILINEAR    <=== BEST

@param im interpolation method `bicubic` or `bilinear`
@param fs filter shape `gaussian` or `ideal`

@return 0 upon success, -1 for invalid interpolation method, -2 for invalid filter shape
*/
int Downsample::set_interpolationMethodAndFilterShape( const std::string im, const std::string fs )
{
  // Interpolation mask and filter shape BOTH specified by config.
  if( ( im != "" ) && ( fs != "" ) )
  {
    _filterShape = fs;
    _configRecap = "Filter shape and interpolation methods specified by user (per config).";
    if( im == "bicubic" )
      _interpolationMethod = cv::INTER_CUBIC;
    else if( im == "bilinear" )
      _interpolationMethod = cv::INTER_LINEAR;
    else
      return -1;

    if( fs == "ideal" )
      _filterShape = "ideal";
    else if( fs == "gaussian" )
      _filterShape = "gaussian";
    else
      return -2;
  }

  // Set params per best experimental results.
  if( ( im == "" ) && ( fs == "" ) )
  {
    _configRecap = "Using recommended filter shape and interpolation method.";
    // Set based on best experimental results
    switch ( get_srcSampleRate() ) {
      case 600:
        _filterShape = "ideal";
        _interpolationMethod = cv::INTER_CUBIC;
        break;

      case 1000:
        _filterShape = "ideal";
        _interpolationMethod = cv::INTER_LINEAR;
        break;

      case 1200:
        _filterShape = "gaussian";
        _interpolationMethod = cv::INTER_LINEAR;
        break;

      default:
        _filterShape = "ideal";
        _interpolationMethod = cv::INTER_LINEAR;
        break;
    }
  }
  return 0;
}

/*
@return "gaussian" or "ideal"
*/
std::string Downsample::get_filterShape(void) const
{
  return _filterShape;
}

}   // End namespace

/** Wrapper for OpenCV `mulSpectrums` function.  THIS IS THE FILTER by multiplication
in the frequency domain.

@param complexI has 2 channels
@param mask has one channel

@return filtered image in freq domain
*/
cv::Mat applyFilterFreqDomain( cv::Mat complexI, cv::Mat mask )
{
  cv::Mat planes[] = {cv::Mat::zeros( complexI.size(), CV_32F ),
                      cv::Mat::zeros( complexI.size(), CV_32F )};
  cv::Mat kernel_spec;
  planes[0] = mask; // real
  planes[1] = mask; // imaginary, no apparent change with this in or out.
  cv::merge( planes, 2, kernel_spec );

  cv::Mat filteredSpectrum;
  cv::mulSpectrums( complexI, kernel_spec, filteredSpectrum, cv::DFT_ROWS );  // DFT_ROWS accepted);

  return filteredSpectrum;
}
