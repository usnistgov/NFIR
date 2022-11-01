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

std::string printVersion(void);
void resample( uint8_t* srcImage, uint8_t** tgtImage,
               int srcSampleRate, int tgtSampleRate, const std::string &,
               std::string interpolationMethod, const std::string &,
               uint32_t *imageWidth, uint32_t *imageHeight,
               size_t *imgBufSize,
               const std::string &, const std::string &,
               std::vector<std::string> & );

void get_filteredImage( uint8_t** tgtImage,
                        const std::string &encodeCompression,
                        size_t   *imgBufSize,
                        uint32_t *imageWidth,
                        uint32_t *imageHeight );
}
