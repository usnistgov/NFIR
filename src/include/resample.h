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
// #ifdef _WIN32
//   #pragma warning( disable: 4514 )  // message(void): unreferenced inline function has been removed.
// #endif                              // Seems that latest version (since 20 Dec 2020) of VS has fixed this.

#include "exceptions.h"
#include "filter_mask.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <string>

// Type alias.
typedef int InterpolationMethod;

/** @brief Applied to source image prior to DFT
 * 
 * Top and left sides are always zero.
 */
struct Padding {
  int top;
  int bottom;
  int left;
  int right;

  /** @brief Reset all four sides to zero, especially right and bottom
   */
  void reset(void)
  {
    top = 0;
    bottom = 0;
    left = 0;
    right = 0;
  }

  /** @brief Image padding for current downsample process
   * @return std::string for entry into metadata
   */
  std::string to_s(void)
  {
    std::string s{"Image Padding: "};
    s.append(   "top  : " + std::to_string(top) );
    s.append( ", left : " + std::to_string(left) );
    s.append( ", bot  : " + std::to_string(bottom) );
    s.append( ", right: " + std::to_string(right) );
    s.append( "\n" );
    return s;
  }
};


namespace NFIR {

/**
 * @brief Base class that is used via polymorphism to support resampling
 *
 * Is never directly instantiated.
 */
class Resample
{
private:

protected:
  /** @brief Resample configuration message for logging */
  std::string _configRecap;

  /**
   * @brief This image has been filtered but not yet downsampled.
   *
   * Its dimensions are width and height of the zero-padded image after crop
   * of the padding.
   * This image is made available to the using software as proof that the
   * lowpass filter has removed all image frequency components above the
   * desired sample rate.
   * See screen-shots of SIVV 1D power spectrum in README.
   */
  cv::Mat _filteredImagePriorToDownsample;
  uint32_t *_filteredImageDimens;

  /** @brief Bicubic or bilinear used to resize source image to target image */
  InterpolationMethod _interpolationMethod;
  /** @brief Calculated, is used to resize source image to target image */
  double _resizeFactor;
  /** @brief User must specify source image resolution in pixels per inch */
  int _srcSampleRate;
  /** @brief User must specify target image resolution in pixels per inch */
  int _tgtSampleRate;

  /** @brief Initialization function that resets all values */
  void Init();

  /** @brief Copy function to make clones of an object */
  void Copy( const Resample& );

public:
  /** @brief Default constructor never used */
  Resample();

  /** @brief Copy constructor */
  Resample( const Resample& );

  /** @brief Full constructor with all accessible members defined */
  Resample( int, int );

  /** @brief Virtual destructor */
  virtual ~Resample() {}


  // Overrides in derived class.
  /** @brief Resize an upsampled image */
  virtual cv::Mat resize( cv::Mat );
  /** @brief Resize a downsampled image */
  virtual cv::Mat resize( cv::Mat, NFIR::FilterMask*, Padding& );
  /** @brief Resample runtime log */
  virtual std::vector<std::string> to_s(void) const;

  // Declare set methods.
  /** @brief Set upsample interpolation */
  virtual void set_interpolationMethod( const std::string );
  /** @brief Set downsample interpolation and low-pass filter type */
  virtual void set_interpolationMethodAndFilterType( const std::string,
                                                      const std::string );
  /** @brief Source image sample-rate/resolution as specified by user */
  void set_srcSampleRate( const int& );
  /** @brief Target image sample-rate/resolution as specified by user */
  void set_tgtSampleRate( const int& );


  // Data get functions cannot modify the object, all declared as const.
  /** @brief Getter method */
  InterpolationMethod get_interpolationMethod(void) const;
  /** @brief Getter method */
  double get_resizeFactor(void) const;
  /** @brief Getter method */
  int get_srcSampleRate(void) const;
  /** @brief Getter method */
  int get_tgtSampleRate(void) const;

  // Implement a clone operator.
  // Resample Clone(void);

  // Implement an assigment operator.
  // Resample operator=( const Resample& );

};

}   // End namespace
