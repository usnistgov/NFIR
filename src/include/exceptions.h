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
#ifdef WIN32
  #pragma warning( disable: 4514 )  // message(void): unreferenced inline function has been removed.
#endif                              // Seems that latest version (since 20 Dec 2020) of VS has fixed this.

#include <exception>
#include <string>

namespace NFIR {

/**
 * @brief Handle exceptions thrown by the Resample constructor and subsequent
 * method calls.
*/
class Miscue: public std::exception {

  /** @brief Custom error message */
  std::string _msg{""};

public:
  /** @brief Constructor
   * @param msg custom error message
   */
  Miscue( const std::string &msg ) : _msg{"NFIR Exception: " + msg} {}
  ~Miscue() {}

  /** Utilize custom error message.
   *
   * @return text of the error message
   */
  std::string message() const
  {
    return "NFRL Exception: " + _msg;
  }

  /** @brief Override the std::exception method
   * @return text of the error message
   */
  const char* what() const noexcept override
  {
    return _msg.c_str();
  }
};

}   // End namespace
