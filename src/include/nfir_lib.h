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

#include "exceptions.h"

#include <string>

#define NFIR_VERSION "0.2.0"

namespace NFIR {

/**
 * @return versions of NFIR, NFIMM, and OpenCV 
 */
std::string
printVersion(void);

/**
 * @return version of NFIR only
 */
std::string
getVersion(void);

/**
 * @brief Primary API to the resampler process that generates a new image
 * at the desired sample rate.
 *
 * This method is the interface into the NIST Fingerprint Image Resampler library.
 * The caller only need use this method to perform the entire, resample process
 * on a single image.
 *
 * It instantiates the up- or down-sample object based on the source and target
 * sample rates.
 *
 * The up- or down-sample *resize factor* = (target rate)/(source rate).
 *
 * For upsample: perform the resize and return.
 *
 * For downsample:
 * - pad the source image
 * - build the lowpass filter
 * - Fourier-transform the padded image
 * - matrix multiply with the filter
 * - Inverse-Fourier transform the product
 * - strip the image padding
 * - perform final resize
 *
 * OpenCV requires that the source image must be 8-bit, single-channel. Attempts
 * are made to convert the source image to single-channel, if necessary, and
 * update the log if conversion was performed.  In the event that this function
 * returns unsuccesfully and without throwing exception, check the number of
 * channels and pixel bit-depth of the source image.
 */
void
resample( uint8_t *, uint8_t **,
               int, int, const std::string &,
               const std::string &, const std::string &,
               uint32_t *, uint32_t *,
               size_t *,
               const std::string &, const std::string &,
               std::vector<std::string> &,
               std::vector<std::string> & );

/**
 * @brief Additional API to get the filtered image prior to downsample.
 *
 * This intermediate image has been cropped of its zero-padding but not (yet)
 * been downsampled.  It is encoded per the compression parameter.  The encoded
 * image is "vectorized" for return.
 *
 * @param filteredImage OUT pointer to image
 * @param encodeCompression desired compression of filteredImage
 * @param imgBufSize OUT size of filteredImage array
 * @param imageWidth OUT pixels
 * @param imageHeight OUT pixels
 */

void
get_filteredImage( uint8_t** tgtImage,
                        const std::string &encodeCompression,
                        size_t   *imgBufSize,
                        uint32_t *imageWidth,
                        uint32_t *imageHeight );
}
