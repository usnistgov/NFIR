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

/** Set source and target rates to 500PPI, mask radius factor to 1.0. */
void FilterMask::Init()
{
  _srcSampleRate = 500;
  _tgtSampleRate = 500;
  _maskRadiusFactor = 1.0;
}

/**
 * @param aCopy copy from this instance
 */
void FilterMask::Copy( const FilterMask& aCopy )
{
  // Init();

  _srcSampleRate = aCopy._srcSampleRate;
  _tgtSampleRate = aCopy._tgtSampleRate;
  _maskRadiusFactor = aCopy._maskRadiusFactor;
}

FilterMask::FilterMask()
{
  Init();
}

/** @param aCopy the instance to copy from */
FilterMask::FilterMask( const FilterMask& aCopy )
{
  Copy( aCopy );
}

/**
 * @param srcSampleRate source image ppi to be resampled
 * @param tgtSampleRate target image ppi of resulting image
 */
FilterMask::FilterMask( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _maskRadiusFactor = (float)_tgtSampleRate / (float)_srcSampleRate;
}


/** Methods are declared virtual and MUST be overridden in derived class.
 *
 * @return the filter shape
 */
FilterMask::FilterShape FilterMask::get_filterShape(void) const
{
  FilterMask::FilterShape fs{};
  return fs;
}

/** Overridden by base class
 * @param cv::Size of the mask
 */
void FilterMask::build( cv::Size ) { }

/** @return value to configure the mask size */
double FilterMask::get_maskRadiusFactor(void) const
{
  return _maskRadiusFactor;
}

/** @return this instance mask that applies the filter */
cv::Mat FilterMask::get_theFilterMask(void) const
{
  return _theFilterMask;
}

}   // End namespace
