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
#include "filter_mask.h"

namespace NFIR {

// Initialization function that resets all values.
void FilterMask::Init()
{
  _srcSampleRate = 500;
  _tgtSampleRate = 500;
  _maskRadiusFactor = 1.0;
}

// Copy function to make clones of an object.
void FilterMask::Copy( const FilterMask& aCopy )
{
  // Init();

  _srcSampleRate = aCopy._srcSampleRate;
  _tgtSampleRate = aCopy._tgtSampleRate;
  _maskRadiusFactor = aCopy._maskRadiusFactor;
}

// Default constructor.
FilterMask::FilterMask()
{
  Init();
}
// Copy constructor.
FilterMask::FilterMask( const FilterMask& aCopy )
{
  Copy( aCopy );
}

/**
 * @brief Base class constructor.  Calculates the filter/mask radius factor.
 *
 * @param srcSampleRate source image ppi to be resampled
 * @param tgtSampleRate target image ppi of resulting image
 */
FilterMask::FilterMask( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _maskRadiusFactor = (float)_tgtSampleRate / (float)_srcSampleRate;
}


// Methods are declared virtual and MUST be overridden in derived class.
FilterMask::FilterShape FilterMask::get_filterShape(void) const
{
  FilterMask::FilterShape fs{};
  return fs;
}

void FilterMask::build( cv::Size ) { }

double FilterMask::get_maskRadiusFactor(void) const
{
  return _maskRadiusFactor;
}

cv::Mat FilterMask::get_theFilterMask(void) const
{
  return _theFilterMask;
}

// *********
// Always create a virtual destructor- implemented in header file: virtual ~FilterMask() {}.
// *********

}   // End namespace
