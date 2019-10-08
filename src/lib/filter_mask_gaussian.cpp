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
#include "filter_mask_gaussian.h"

#include <opencv2/imgproc/imgproc.hpp>

static void quadrantSwap( cv::Mat );

namespace NFIR {

// Default constructor.
Gaussian::Gaussian()
{
  Init();
}
// Copy constructor.
Gaussian::Gaussian( const Gaussian& aCopy )
{
  Copy( aCopy );
}

/** Full constructor.  Calculates the filter/mask radius factor.
Sets the filter/mask shape to `gaussian`.

@param srcSampleRate source image ppi to be downsampled
@param tgtSampleRate target image ppi of resulting image
*/
Gaussian::Gaussian( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _maskRadiusFactor = (float)_tgtSampleRate / (float)_srcSampleRate;
  _filterShape = FilterShape::gaussian;

  dirty = true;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Ideal() {}.
// *********

FilterMask::FilterShape Gaussian::get_filterShape(void) const
{
  return _filterShape;
}


/**
Gaussian "curve" is based on size of source image in both x- and y- directions.
Uses OpenCV gaussian kernel generator.  Normalize and shift (ie, quadrant swap)
the mask and save it to instance variable.

@param mask_size width and height in pixels
*/
void Gaussian::build( cv::Size mask_size )
{
  double sigmaHeight = _maskRadiusFactor * mask_size.height / 2.0;
  double sigmaWidth = _maskRadiusFactor * mask_size.width / 2.0;

  // OpenCV gaussian kernel generator
  cv::Mat kernelX = cv::getGaussianKernel( mask_size.height, sigmaHeight, CV_32F );
  cv::Mat kernelY = cv::getGaussianKernel( mask_size.width,  sigmaWidth,  CV_32F );

  // create 2d gaus - must transpose the col-vector to row-vector.
  cv::Mat kernel = kernelX * kernelY.t();
  cv::Mat normKernel;
  cv::normalize( kernel, normKernel, 0, 1, cv::NORM_MINMAX );

  quadrantSwap( normKernel );

  _theMask = normKernel.clone();
}

}   // End namespace

/** Rearrange the quadrants of a Fourier image so that the origin is at
the image center. Also known as a "shift".

@param magI fourier "image" to be shifted
*/
void quadrantSwap( cv::Mat magI )
{
  // Rearrange the quadrants of Fourier image so that the origin is at the image center
  int cx = magI.cols/2;
  int cy = magI.rows/2;
  cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp;            // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);     // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);
}
