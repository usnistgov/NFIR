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
/*
Copyright (c) 2013 szx

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <fnmatch.h>
#include "glob_posix.h"

static std::pair<std::string, std::string> SplitPath(const std::string &path) {
  std::string::size_type last_sep = path.find_last_of("/");
  if (last_sep != std::string::npos) {
    return std::make_pair(std::string(path.begin(), path.begin() + last_sep),
                          std::string(path.begin() + last_sep + 1, path.end()));
  }
  return std::make_pair(".", path);
}

namespace glob {

Glob::Glob(const std::string &pattern)
 : dir_(0),
   dir_entry_(0)
{
  std::pair<std::string, std::string> dir_and_mask = SplitPath(pattern);

  dir_ = opendir(dir_and_mask.first.c_str());
  pattern_ = dir_and_mask.second;

  if (dir_ != 0) {
    Next();
  }
}

Glob::~Glob() {
  if (dir_ != 0) {
    closedir(dir_);
  }
}

bool Glob::Next() {
  while ((dir_entry_ = readdir(dir_)) != 0) {
    if (!fnmatch(pattern_.c_str(), dir_entry_->d_name,
                 FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD)) {
      return true;
    }
  }
  return false;
}

} // namespace glob
