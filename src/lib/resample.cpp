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
#include "resample.h"

namespace NFIR {

// Initialization function that resets all values.
void Resample::Init()
{
  _interpolationMethod = cv::INTER_CUBIC;
  _resizeFactor = 1.0;
  _srcSampleRate = 500;
  _tgtSampleRate = 500;
  dirty = false;
}

// Copy function to make clones of an object.
void Resample::Copy( const Resample& aCopy )
{
  // Init();

  _interpolationMethod = aCopy._interpolationMethod;
  _resizeFactor = aCopy._resizeFactor;
  _srcSampleRate = aCopy._srcSampleRate;
  _tgtSampleRate = aCopy._tgtSampleRate;
  dirty = true;
}

// Default constructor.
Resample::Resample()
{
  Init();
}
// Copy constructor.
Resample::Resample( const Resample& aCopy )
{
  Copy( aCopy );
}
// Full constructor with all accessible members defined.
Resample::Resample( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _resizeFactor = (float)_tgtSampleRate / (float)_srcSampleRate;

  dirty = true;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Resample() {}.
// *********

// 'resize()' methods are declared virtual and MUST be overridden in derived class.
cv::Mat Resample::resize( cv::Mat _srcImg ) { cv::Mat mat; return mat; }   // upsample
cv::Mat Resample::resize( cv::Mat _srcImg, NFIR::FilterMask* _filterMask, Padding& _pads )   // downsample
{
  cv::Mat mat;
  return mat;
}
void Resample::to_s() const { }   // print config params to console

// Next, define accessors for all data that can be public. If
// its not intended to be public, make it a private accessor.
// First, define all the set methods. The "dirty" flag is not
// necessary for completeness, but it makes life a LOT easier.

int Resample::set_interpolationMethod( const std::string _s ) { int errorCode{0}; return errorCode; }
int Resample::set_interpolationMethodAndFilterShape( const std::string _im, const std::string _fs )
{
  int errorCode{0};
  return errorCode;
}

void Resample::set_srcSampleRate( const int& x )
{
  if ( _srcSampleRate != x )
    dirty = true;

  _srcSampleRate = x;
}
void Resample::set_tgtSampleRate( const int& x )
{
  if ( _tgtSampleRate != x )
    dirty = true;

  _tgtSampleRate = x;
}


// Now the data get functions. They cannot modify the object as they
// are all marked as const.
InterpolationMethod Resample::get_interpolationMethod(void) const
{
  return _interpolationMethod;
}
int Resample::get_srcSampleRate(void) const
{
  return _srcSampleRate;
}
int Resample::get_tgtSampleRate(void) const
{
  return _tgtSampleRate;
}
double Resample::get_resizeFactor(void) const
{
  return _resizeFactor;
}

std::string Resample::get_filterShape(void) const { std::string s{ "" }; return s; }

}   // End namespace
