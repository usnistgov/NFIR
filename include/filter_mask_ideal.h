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

namespace NFIR {

/** Support the Ideal filter used for downsample.
*/
class Ideal : public FilterMask
{
private:
  bool dirty;       // Keep track of the object state.

  FilterShape _filterShape;

public:
  // Default constructor.
  Ideal();

  // Copy constructor.
  Ideal( const Ideal& );

  // Full constructor.
  Ideal( int , int );

  // Virtual destructor
  virtual ~Ideal() {}

  FilterShape get_filterShape(void) const override;

  /**
  Build the ideal filter that is same size as the source image dimensions (W x H).
  Since this is a low-pass filter, all values within the ellipse are set to **1.0**,
  all those outside set to **0.0**.  This filter is multiplied with the source image
  in the frequency domain.
  */
  void build( cv::Size ) override;


  // Implement a clone operator.
  Ideal Clone(void);

  // Implement an assigment operator.
  Ideal operator=( const Ideal& );
};

}   // End namespace
