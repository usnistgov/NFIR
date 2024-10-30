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

/** Full constructor used by client.
 * Calculate the resize factor to set the size of the target image.
 *
 * @param srcSampleRate pixels per inch
 * @param tgtSampleRate pixels per inch
 */
Downsample::Downsample( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;
  _resizeFactor = (float)_tgtSampleRate / (float)_srcSampleRate;

  _filteredImageDimens = new uint32_t[2];
}

/**
 * The OpenCV resize() function uses the __resize factor__
 *  and interpolation method.
 *
 * @param srcImg to be downsampled
 * @param filterMask that is multiplied with the freq domain image
 * @param pads amount to crop the space domain image
 * @return final downsampled, resized image
 * @throw cv::Exception on any/all possible OpenCV exceptions
 */
cv::Mat Downsample::resize( cv::Mat srcImg,
                            NFIR::FilterMask* filterMask,
                            Padding& pads )
{
  cv::Mat resampledImg;
  try
  {
    // ------ STEP #1) DFT.
    cv::Mat planes[2] = { cv::Mat_<float>(srcImg), cv::Mat::zeros( srcImg.size(), CV_32F ) };
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);

    cv::Mat fourierTransform;
    cv::dft( complexI, fourierTransform, cv::DFT_COMPLEX_OUTPUT );

    // ------ STEP #2) Scale/normalize by dividing by the DC component.
    cv::Mat scaledFwdDFT;
    cv::multiply( fourierTransform, (float)1/srcImg.total(), scaledFwdDFT );

    // ------ STEP #3) Apply filter/mask to image spectrum in freq domain.
    // Multiplication in freq domain where the (multiplicands/factors) spectrums
    // have DC term in center.  The filter/mask was constructed to have DC term
    // in center, therefore, shift the fourier transform signal.
    cv::Mat filteredSpectrum = applyFilterFreqDomain( scaledFwdDFT, filterMask->get_theFilterMask() );

    // ------ STEP #4) Inverse Fourier transform (iDFT).
    cv::Mat inverseTransform;
    cv::dft( filteredSpectrum, inverseTransform, cv::DFT_INVERSE );

    // ------ STEP #5) extract image that is (polar) magnitude of inverse Fourier transform.
    cv::Mat planes2[] = { cv::Mat::zeros( inverseTransform.size(), CV_32F ), cv::Mat::zeros( inverseTransform.size(), CV_32F ) };
    cv::split(inverseTransform, planes2);  // planes2[0] = Re(DFT(I), planes2[1] = Im(DFT(I))
    cv::Mat finalImage;
    planes2[0].convertTo(finalImage, CV_8U);  // cast bit depth for each pixel to 8-bits (0..255) from CV_32F

    // ------ STEP #6) Crop padding.
    int startX=pads.left;  // since padding only right and bottom, this is 0.
    int startY=pads.top;   // since padding only right and bottom, this is 0.
    int cropWidth=srcImg.cols - pads.right;
    int cropHeight=srcImg.rows - pads.bottom;
    cv::Mat croppedImage( finalImage, cv::Rect(startX, startY, cropWidth, cropHeight) );

    _filteredImagePriorToDownsample = croppedImage.clone();
    _filteredImageDimens[0] = _filteredImagePriorToDownsample.cols;
    _filteredImageDimens[1] = _filteredImagePriorToDownsample.rows;

    // ------ STEP #7) Downsize to target ppi.
    cv::resize( croppedImage, resampledImg, cv::Size(0, 0), _resizeFactor, _resizeFactor, _interpolationMethod );
  }
  catch( const cv::Exception& ex ) {
    throw ex;
  }

  // resampledImg.release();  // Force test NFIR::Miscue
  return resampledImg;
}


/**
 * @param srcImg "place-holder"
 * @return source image passed-in
 */
cv::Mat Downsample::resize( cv::Mat srcImg )
{
  return srcImg;
}

/** @return WxH */
uint32_t *Downsample::get_filteredImageDimens() const
{
  return _filteredImageDimens;
}

/** @return image buffer before decimation and after filter */
cv::Mat Downsample::get_filteredImage() const
{
  return _filteredImagePriorToDownsample;
}

/**
 *  @return Downsample copy of this downsample object
 */
Downsample Downsample::Clone(void)
{
  Downsample c;
  c.Copy( *this );
  return c;
}

/**
 * @param aCopy object to be copied
 *  @return Downsample copy of passed object
 */
Downsample Downsample::operator=( const Downsample& aCopy )
{
  Copy( aCopy );
  return *this;
}


std::vector<std::string> Downsample::to_s(void) const
{
  std::vector<std::string> v;
  v.push_back("DOWNSAMPLE configuration:");
  v.push_back("  source sample rate:  " + std::to_string(get_srcSampleRate()) );
  v.push_back("  target sample rate:  " + std::to_string(get_tgtSampleRate()) );
  v.push_back("  resize factor:       " + std::to_string(get_resizeFactor()) );
  v.push_back("Filter & Interpolation: " + _configRecap );
  v.push_back("  filter/mask type:     " + _filterType );
  v.push_back("  interpolation method:  " + std::to_string(get_interpolationMethod())
            + "  (1=bilinear, 2=bicubic)\n" );
  return v;
}

/**
 * @param im interpolation method `bicubic` or `bilinear`
 * @param fs filter type `Gaussian` or `ideal`
 *
 * @throw NFIR::Miscue invalid interpolation method or filter type
 */
void Downsample::set_interpolationMethodAndFilterType( const std::string im, const std::string fs )
{
  // Interpolation and filter type BOTH specified by config.
  if( ( im != "" ) && ( fs != "" ) )
  {
    _filterType = fs;
    _configRecap = "Filter type and interpolation methods specified by user (per config).";
    if( im == "bicubic" )
      _interpolationMethod = cv::INTER_CUBIC;
    else if( im == "bilinear" )
      _interpolationMethod = cv::INTER_LINEAR;
    else
      throw NFIR::Miscue( "NFIR lib: invalid interpolation method: " + im );

    if( fs == "ideal" )
      _filterType = "ideal";
    else if( ( fs == "gaussian" ) || ( fs == "Gaussian" ) )
      _filterType = "Gaussian";
    else
      throw NFIR::Miscue( "NFIR lib: invalid filter type: " + fs );
  }

  // Set params per best experimental results.
  if( ( im == "" ) && ( fs == "" ) )
  {
    _configRecap = "Using recommended filter type and interpolation method.";
    // Set based on best experimental results
    switch ( get_srcSampleRate() ) {
      case 600:
        _filterType = "ideal";
        _interpolationMethod = cv::INTER_CUBIC;
        break;

      case 1000:
        _filterType = "ideal";
        _interpolationMethod = cv::INTER_LINEAR;
        break;

      case 1200:
        _filterType = "Gaussian";
        _interpolationMethod = cv::INTER_LINEAR;
        break;

      default:
        _filterType = "ideal";
        _interpolationMethod = cv::INTER_LINEAR;
        break;
    }
  }
}

/** @return "Gaussian" or "ideal" */
std::string Downsample::get_filterType(void) const
{
  return _filterType;
}

}   // End namespace

/**
 * @brief Wrapper for OpenCV `mulSpectrums` function.
 *
 * IMPLEMENT THE LOWPASS FILTER by multiplication in the frequency domain.
 *
 * Since the lowpass filter mask was constructed in one "channel" as a single
 * 2-dimensional array (a "plane"), the mask is "merged" into 2 channels.
 *
 * ```
 *  EXAMPLE]
 *
 *  Mat m1 = (Mat_<uchar>(2,2) << 1,4,7,10);
 *  Mat m2 = (Mat_<uchar>(2,2) << 2,5,8,11);
 *  Mat m3 = (Mat_<uchar>(2,2) << 3,6,9,12);
 *  Mat channels[3] = {m1, m2, m3};
 *  Mat m;
 *  merge(channels, 3, m);
 *  ---
 *  m =
 *    [1, 2, 3, 4,  5,  6;
 *     7, 8, 9, 10, 11, 12]
 *  m.channels() = 3
 *  ---
 * ```
 *
 * Only the real-part of the complex image is filtered, so half the merged
 * array contains zeros "interlaced" with the real-part.  The 2-channel complex
 * image is multiplied by this 2-channel mask.
 *
 * @param complexI has 2 channels
 * @param mask has one channel
 *
 * @return filtered image in freq domain
 */
cv::Mat applyFilterFreqDomain( cv::Mat complexI, cv::Mat mask )
{
  cv::Mat planes[] = {cv::Mat::zeros( complexI.size(), CV_32F ),
                      cv::Mat::zeros( complexI.size(), CV_32F )};
  cv::Mat kernel_spec;
  planes[0] = mask;    // real
  // planes[1] = mask; // imaginary, no change with this in or out.
  cv::merge( planes, 2, kernel_spec );

  cv::Mat filteredSpectrum;
  cv::mulSpectrums( complexI, kernel_spec, filteredSpectrum, cv::DFT_ROWS );  // DFT_ROWS accepted);

  return filteredSpectrum;
}
