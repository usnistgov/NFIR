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

#include <opencv2/core/core.hpp>

namespace NFIR {

/**
 * @brief Base class that is used via polymorphism to support Ideal and Gaussian
 * filter/masks.  Is never directly instantiated.
 */
class FilterMask
{
public:
  /**
   * @brief Filter shapes used for downsample (only)
   *
   * Ideal and Gaussian (no Butterworth).
   */
  enum class FilterShape : unsigned
  {
    ideal,
    Gaussian
  };


protected:
  /** @brief (float)_tgtSampleRate / (float)_srcSampleRate */
  double _maskRadiusFactor;
  /** @brief The low pass filter */
  cv::Mat _theFilterMask;

  /** @brief source image resolution */
  int _srcSampleRate;
  /** @brief target image resolution */
  int _tgtSampleRate;

  /** @brief Initialization function that resets all values */
  void Init();

  /** @brief Copy function to make clones of an object */
  void Copy( const FilterMask& );

public:
  /** @brief Default constructor never used */
  FilterMask();

  /** @brief Copy constructor */
  FilterMask( const FilterMask& );

  /** @brief Base class constructor calculates the filter/mask radius factor */
  FilterMask( int, int );

  /** @brief Virtual destructor */
  virtual ~FilterMask() {}

  // Override in derived class.
  /** @brief Ideal or Gaussian */
  virtual FilterShape get_filterShape(void) const;
  /** @brief Build the filter/mask; implemented in subclass */
  virtual void build( cv::Size );

  /** @brief Setter method */
  void set_srcSampleRate( const int& );
  /** @brief Setter method */
  void set_tgtSampleRate( const int& );


  /** @brief Getter method */
  int get_srcSampleRate(void) const;
  /** @brief Getter method */
  int get_tgtSampleRate(void) const;
  /** @brief tgtSampleRate / srcSampleRate */
  double get_maskRadiusFactor(void) const;
  /** @brief Getter method */
  cv::Mat get_theFilterMask(void) const;

  /** @brief Implement a clone operator */
  FilterMask Clone(void);

  /** @brief Implement an assigment operator */
  FilterMask operator=( const FilterMask& );
};

}   // End namespace
