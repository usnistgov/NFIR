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
#include "filter_mask_ideal.h"

#include <opencv2/imgproc/imgproc.hpp>

namespace NFIR {

// Default constructor.
Ideal::Ideal()
{
  Init();
}
// Copy constructor.
Ideal::Ideal( const Ideal& aCopy ) : FilterMask::FilterMask( aCopy )
{
  Copy( aCopy );
}

/** Full constructor.  Calculates the filter/mask radius factor.
Sets the filter/mask shape to `ideal`.

@param srcSampleRate source image ppi to be downsampled
@param tgtSampleRate target image ppi of resulting image
*/
Ideal::Ideal( int srcSampleRate, int tgtSampleRate )
{
  _srcSampleRate = srcSampleRate;
  _tgtSampleRate = tgtSampleRate;

  _maskRadiusFactor = (float)_tgtSampleRate / (float)_srcSampleRate;
  _filterShape = FilterShape::ideal;

  dirty = true;
}


// *********
// Always create a virtual destructor- implemented in header file: virtual ~Ideal() {}.
// *********


FilterMask::FilterShape Ideal::get_filterShape(void) const
{
  return _filterShape;
}

/**
Use Meshgrid for mask shape using the ellipse.
Per "classical" ellipse theory, the major-axis is the longer of the two.
For example, for an image/mask that is wider than higher, the x-axis is
the major-axis and the y-axis is the minor axis.
This makes the ellipse equation:

  `x^2/a^2  +  y^2/b^2  = 1   when a > b`

However, using the meshgrid technique, there is no need to keep track of
major/minor axes.

Simply scale the meshgrid arrays by the ellipse horizontal (a) and vertical (b)
vertices.  The vertices are one-half the width and height of the 2-dimensional
mask (where the mask is same cv::Size as source image).

Calculate the magnitude of the scaled meshgrids using cv::magnitude(a,b).
It is this `meshgridScaledMagnitude' array that is "traversed" and "checked"
against a "distance" -OR- cut-off frequency.

Save filter/mask to instance variable.

@param mask_size `width` x `height`
*/
void Ideal::build( cv::Size mask_size )
{
  // M x N  :  width x height
  int M = mask_size.width;     // count cols
  int N = mask_size.height;    // count rows

  cv::Mat meshgridRows = cv::Mat( N, M, CV_32F );
  cv::Mat meshgridCols = cv::Mat( N, M, CV_32F );

  for(int i=0; i<N; i++)
  {
    for(int j=0; j<M; j++)
    {
      if(j <= M/2)
        meshgridRows.at<float>(i,j) = (float)(j);
      else
        meshgridRows.at<float>(i,j) = (float)(j) - M;
    }
  }

  for(int i=0; i<N; i++)
  {
    for(int j=0; j<M; j++)
    {
      if(i <= N/2)
        meshgridCols.at<float>(i,j) = (float)(i);
      else
        meshgridCols.at<float>(i,j) = (float)(i) - N;
    }
  }
 
  // Calc distance as Ellipse
  cv::Mat meshgridScaledMagnitude;  // this is OK,  = cv::Mat(N, M, CV_32F);
  cv::Mat meshgridScaledRows, meshgridScaledCols;

  // Get the vertices based on the dimensions of the mask.
  double aVertex, bVertex;
  aVertex = mask_size.width / (2.0 * _maskRadiusFactor);
  bVertex = mask_size.height / (2.0 * _maskRadiusFactor);

  // Next, scale the meshgrid arrays by the ellipse horizontal (a) and vertical (b)
  // vertices.  The vertices are one-half the width and height of the 2-dimensional
  // mask (where the mask is same size as source image).
  cv::multiply( bVertex, meshgridRows, meshgridScaledRows );
  cv::multiply( aVertex, meshgridCols, meshgridScaledCols );
  cv::magnitude( meshgridScaledRows, meshgridScaledCols, meshgridScaledMagnitude );

  // Find the maximum value in row(0); this either of the semi-major axis
  // or semi-minor axis.  It's the major axis if the image width is greater
  // than the height.  However, based on this technique, this is no longer
  // a concern.
  double rowMin, rowMax;
  cv::minMaxLoc( meshgridScaledMagnitude.row(0), &rowMin, &rowMax );

  // Find the maximum value in col(0); this either of the semi-major axis
  // or semi-minor axis.  It's the major axis if the image height is greater
  // than the width.  However, based on this technique, this is no longer
  // a concern.
  double colMin, colMax;
  cv::minMaxLoc( meshgridScaledMagnitude.col(0), &colMin, &colMax );

  // Finally, the "radius" is the smaller of the semi-major axes.
  // The desired filter cutoff, ie Nyquist, is then 1/2 of the "radius".
  // Because of this technique, the "max" value is IDENTICAL in both
  // the zero-row and zero-column.  However, this code was left intact
  // since the IDENTICAL values were only discovered to be so upon testing.
  double distDiscriminator = std::min( rowMax, colMax ) * _maskRadiusFactor;

  // Set mask value based on distance-frequency (cutoff distance).
  cv::Mat tmp = cv::Mat( N, M, CV_32F );
  for(int i=0; i<N; i++)
  {
    for(int j=0; j<M; j++)
    {
      if(meshgridScaledMagnitude.at<float>(i,j) < distDiscriminator)
      {
        tmp.at<float>(i,j) = (float)(1.0);
      }
      else
      {
        tmp.at<float>(i,j) = (float)(0.0);
      }
    }
  }
  _theMask = tmp.clone();
}

}   // End namespace
