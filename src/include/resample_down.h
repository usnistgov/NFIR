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

#include "resample.h"

namespace NFIR {

/** @brief Support downsample process. */
class Downsample : public Resample
{
private:
  std::string _filterShape;

public:
  // Default constructor.
  Downsample();

  // Copy constructor.
  Downsample( const Downsample& );

  /** @brief Full constructor.
   *
   * Calculates the image resize factor.
   *
   * @param srcSampleRate source image ppi to be downsampled
   * @param tgtSampleRate target image ppi of resulting downsample
   */
  Downsample( int, int );

  // Virtual destructor
  virtual ~Downsample() { delete [] _filteredImageDimens; }


  /** @brief Used to upsample. */
  cv::Mat resize( cv::Mat ) override;

  /**
   * @brief Pass-in the padded source image and the lowpass filter mask.
   *
   * Note that the filter dimensions is the same as the padded image.  Since
   * the filter is already built, the source image is the one that is padded.
   * The padding must therefore be a passed-in parameter to perform the final
   * cropping.
   *
   * Includes image processing prior to the OpenCV `resize()` function call:
   *
   * 1. Calculate the Discrete Fourier Transform (DFT) of the `srcImg`
   * 2. Factor-out the DC component (by dividing each freq by size of `srcImg`)
   * 3. Apply the filter/mask
   * 4. Calulate the inverse DFT
   * 5. Extract image that is (polar) magnitude of inverse DFT
   * 6. Crop space domain image
   * 7. Call the OpenCV `resize` function.
   *
   * The OpenCV resize() function uses the __resize factor__
   *  and interpolation method.
   *
   * @param srcImg to be downsampled
   * @param filterMask that is multiplied with the freq domain image
   * @param pads amount to crop the space domain image
   *
   * @return final downsampled, resized image
   */
  cv::Mat resize( cv::Mat, NFIR::FilterMask*, Padding& ) override;
  std::vector<std::string> to_s(void) const override;

  /**
   * @brief Implements the recommended filter shape and interpolation method
   *  if these config params not set by user.
   *
   *  See also FilterMask::FilterShape.
   *
   * **IDEAL LPF, use interpolation:
   *
   *      600 to 500ppi - BICUBIC     <=== BEST - set by this method
   *     1000 to 500ppi - BILINEAR    <=== BEST and default
   *     1200 to 500ppi - BILINEAR    <=== MANUAL - must specify per config params
   *
   * **Gaussian LPF, use interpolation:
   *
   *      600 to 500ppi - BICUBIC     <=== MANUAL
   *     1000 to 500ppi - BICUBIC     <=== MANUAL
   *     1200 to 500ppi - BILINEAR    <=== BEST
   *
   * @param im interpolation method `bicubic` or `bilinear`
   * @param fs filter shape `Gaussian` or `Ideal`
   *
   * @throw NFIR::Miscue invalid interpolation method or filter shape
   */
  void set_interpolationMethodAndFilterShape( const std::string,
                                              const std::string ) override;

  /** @return "Gaussian" or "ideal" */
  std::string get_filterShape(void) const;

  /** @brief This image is made available as 'optional'.
   *
   * It is not required to keep or maintain this image for the downsample
   * process.  The spectral power of this image is "cut off" at the target
   * sample rate and is made obvious by SIVV's Log Power Spectrum graph.
   * See https://doi.org/10.6028/NIST.IR.7968.
   *
   * @return target image that is filtered but not yet resampled
   */
  cv::Mat get_filteredImage(void) const;

  /** @return WxH */
  uint32_t *get_filteredImageDimens(void) const;

  // Implement a clone operator.
  Downsample Clone(void);

  // Implement an assigment operator.
  Downsample operator=( const Downsample& );
};

}   // End namespace
