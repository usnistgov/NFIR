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
#pragma once

#include "filter_mask.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <string>

// Type alias.
typedef int InterpolationMethod;

struct Padding {
  unsigned top;
  unsigned bottom;
  unsigned left;
  unsigned right;
};


namespace NFIR {

class Resample
{
private:
  bool dirty;       // Keep track of the object state.

protected:
  std::string _configRecap;
  InterpolationMethod _interpolationMethod;
  double _resizeFactor;
  int _srcSampleRate;
  int _tgtSampleRate;

  // Initialization function that resets all values.
  void Init();

  // Copy function to make clones of an object.
  void Copy( const Resample& );

public:
  // Default constructor.
  Resample();

  // Copy constructor.
  Resample( const Resample& );

  // Full constructor with all accessible members defined.
  Resample( int, int );

  // Virtual destructor
  virtual ~Resample() {}


  // Overrides in derived class.
  virtual cv::Mat resize( cv::Mat );   // Upsample
  virtual cv::Mat resize( cv::Mat, NFIR::FilterMask*, Padding& );   // Downsample
  virtual void to_s(void) const;

  // Next, define accessors for all data that can be public. If
  // its not intended to be public, make it a private accessor.
  // First, define all the set methods. The "dirty" flag is not
  // necessary for completeness, but it makes life a LOT easier

  virtual int set_interpolationMethod( const std::string );
  virtual int set_interpolationMethodAndFilterShape( const std::string, const std::string );
  void set_srcSampleRate( const int& );
  void set_tgtSampleRate( const int& );


  // Now the data get functions. They cannot modify
  // the object, they are all marked as const.
  virtual std::string get_filterShape(void) const;
  InterpolationMethod get_interpolationMethod(void) const;
  double get_resizeFactor(void) const;
  int get_srcSampleRate(void) const;
  int get_tgtSampleRate(void) const;

  // Implement a clone operator.
  Resample Clone(void);

  // Implement an assigment operator.
  Resample operator=( const Resample& );

};

}   // End namespace
