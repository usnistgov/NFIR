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
#include "CLI11.hpp"
#include "glob.h"
#include "nfir_lib.h"


#include <chrono>
#include <ctime>
#include <stdexcept>
#include <string>


static int srcSampleRate{0};
static int tgtSampleRate{0};

static std::string buildTargetImageFilename( std::string );
static cv::Mat readImage( std::string );
static void retrieveSourceImagesList( std::string, std::string, std::vector<std::string>& );

inline char separator()
{
#ifdef _WIN32_64
  return '\\';
#else
  return '/';
#endif
}


/**
Process the command-line or config file runtime parameters.  Based on the source
and target sample rates, run either up- or down-sample.

For downsample, the filter/mask is built for each image (based on width/height of image).
*/
int main(int argc, char** argv) 
{
  CLI::App app{"Upsample, downsample images with known ppi. Command line options override config(.ini) file settings."};

  CLI::Option *cf_opt = app.set_config(
                "-u, --use-config",
                "",
                "Use config file instead of cmd line switches",
                false);
  // app.allow_config_extras(true);


  app.add_option( "-a, --src-samp-rate", srcSampleRate, "Source imagery sample rate" );
  app.add_option( "-b, --tgt-samp-rate", tgtSampleRate, "Target imagery sample rate" );
    // ->required();

  std::string srcFile {};
  CLI::Option *sf_opt = app.add_option( "-c, --src-file", srcFile, "Source imagery file (absolute or relative)" )
    ->check(CLI::ExistingFile);
  std::string tgtFile {};
  app.add_option( "-d, --tgt-file", tgtFile, "Target imagery file (absolute or relative)" )
    ->needs(sf_opt);
  
  std::string srcDir {};
  app.add_option( "-s, --src-dir", srcDir, "Source imagery dir (absolute or relative)" )
    ->check(CLI::ExistingDirectory);
  std::string tgtDir {};
  app.add_option( "-t, --tgt-dir", tgtDir, "Target imagery dir (absolute or relative)" )
    ->check(CLI::ExistingDirectory);

  std::string imageFileFormat {"png"};
  app.add_option( "-m, --img-fmt", imageFileFormat, "Image compression format by filename extension, default is 'png'" );

  std::string interpolationMethod {};
  CLI::Option *im_opt = app.add_option( "-i, --interp-method", interpolationMethod, "For interpolation use [ bicubic | bilinear ]" );

  std::string filterShape {};
  CLI::Option *fs_opt = app.add_option( "-f, --downsamp-filter-shape", filterShape, "For filter use [ ideal | gaussian ]" )
    ->needs(im_opt);
  im_opt->needs(fs_opt);

  bool dryRunFlag {false};
  app.add_flag( "-x,--dry-run", dryRunFlag, "Do not write images to disk" )
    ->multi_option_policy()
    ->ignore_case();

  bool verifyFlag {true};
  app.add_flag( "-y,--verify", verifyFlag, "Print config params prior to resampling; prompt to continue" )
    ->multi_option_policy()
    ->ignore_case();
    // ->take_last(false);  // DOES NOT COMPILE; only allowed once on cmd line

  bool verboseFlag {false};
  app.add_flag( "-z,--verbose", verboseFlag, "Print target file path and final runtime count" )
    ->multi_option_policy()
    ->ignore_case();

  bool versionFlag {false};
  app.add_flag( "-v,--version", versionFlag, "Print NFIR, OpenCV versions and exit" )
    ->multi_option_policy()
    ->ignore_case();

  bool printConfigFlag {false};
  app.add_flag( "-p,--print-config", printConfigFlag, "Print config file and exit" )
    ->multi_option_policy()
    ->needs(cf_opt)
    ->ignore_case();

  app.get_formatter()->column_width(20);

  // If cmd line has zero switches, force -h.
  if( argc==1 )
  {
    char hlp[8] = {"--help\0"};
    argv[argc++] = hlp;
  }

  // This is what the macro CLI11_PARSE expands to:
  // CLI11_PARSE(app, argc, argv);
  try
  {
    app.parse( argc, argv );
  }
  catch( const CLI::ParseError &e )
  {
    app.exit(e);
    return -1;
  }




  if( versionFlag )
  {
    NFIR::printVersion();
    return(0);
  }

  if( printConfigFlag )
  {
    std::cout << "*** Specified config file content ***" << std::endl;
    std::cout << app.config_to_str(false, true) << std::endl;
    return(0);
  }

  // Output config data to console and prompt to continue.
  if( verifyFlag )
  {
    NFIR::printVersion();
    std::cout << "  *** Verify runtime parameters ***" << std::endl;
    std::cout << "Source sample rate: '" << srcSampleRate << "'" << std::endl;
    std::cout << "Target sample rate: '" << tgtSampleRate << "'" << std::endl;
    std::cout << "Source image file: '"  << srcFile << "'" << std::endl;
    std::cout << "Target image file: '"  << tgtFile << "'" << std::endl;
    std::cout << "Source imagery dir: '" << srcDir  << "'" << std::endl;
    std::cout << "Target imagery dir: '" << tgtDir  << "'" << std::endl;
    std::cout << std::endl;
    std::cout << "Dry-run: " << std::boolalpha << dryRunFlag << std::endl;
    std::cout << "Verbose mode: " << std::boolalpha << verboseFlag << std::endl;

    char key_press{};
    bool loooop{ true };
    std::cout << "Press y to continue, n to exit:  ";
    while( loooop )
    {
      std::cin >> key_press;
      switch( key_press )
      {
        case 'y':
          loooop = false;
          break;
        case '\n':       // otherwise, line is dumped twice.
          std::cout << "Try again:  ";
          loooop = true;
          break;
        case 'n':
          exit(0);
      }
    }
  }

  int tmp_count{0};
  std::vector<std::string> listSrcImages{};
  if( srcFile != "" ) {
    listSrcImages.push_back( srcFile );
  }
  else {
    retrieveSourceImagesList(srcDir, imageFileFormat, listSrcImages);
  }

  auto startStamp = std::chrono::system_clock::now();


  // START LOOP through all src images.
  std::string tgtPath{""};
  cv::Mat srcImage,tgtImage;

  for( auto it:listSrcImages )
  {
    if( srcFile != "" ) {
      tgtPath = tgtFile;
    }
    else {
      std::string tgtFname = buildTargetImageFilename( it );
      tgtPath = tgtDir + tgtFname;
    }

    srcImage.release();
    srcImage = readImage( it );
    if( srcImage.empty() )
    {
      std::cout << "Cannot open image: '" << it << "'\n";
      continue;
    }

    // Perform the resample by calling the library's resample() method.
    if( !dryRunFlag )
    {
      try {
        NFIR::resample( srcImage, tgtImage,
                  srcSampleRate, tgtSampleRate,
                  interpolationMethod, filterShape );
        cv::imwrite( tgtPath, tgtImage );
        tmp_count += 1;
      }
      catch( const std::runtime_error& e ) {
        std::cerr << "resample() runtime error: " << e.what() << std::endl;
        return -1;
      }
      catch( const std::invalid_argument& e ) {
        std::cout << "resample() invalid argument: " << e.what() << std::endl;
        return -1;
      }
      catch( const cv::Exception& ex ) {
        std::cout << "NFIR bin: Exception for '" << it << "'.\n  OpenCV error message: "
                  << ex.what() << "Image format attempted: " << imageFileFormat << "\n\n";
        continue;
      }
    }

    if( verboseFlag )
    {
      if( dryRunFlag )
        std::cout << "dry-run tgtPath: " << tgtPath << std::endl ;
      else
        std::cout << "tgtPath: " << tgtPath << std::endl ;
    }

  }   // END LOOP through all src images.

  auto endStamp = std::chrono::system_clock::now();
  std::time_t endTime = std::chrono::system_clock::to_time_t( endStamp );

  std::chrono::duration<double> elapsedSeconds{ endStamp-startStamp };

  std::cout << "Total RESAMPLED images count: " << tmp_count << std::endl;
  std::cout << "Finished computation at " << std::ctime(&endTime)
            << "Elapsed time: " << elapsedSeconds.count() << "s\n";
  return 0;
}


/**
Build string: `-srcSampleRateStr + "to" + tgtSampleRateStr + "ppi"` and append
to source fname.

@param srcPath path of source image

@return filename of target image
*/
std::string buildTargetImageFilename( std::string srcPath )
{
  std::string out = "";

  size_t found;
  found = srcPath.find_last_of( "/\\" );
  std::string folder = srcPath.substr(0,found);
  std::string fname = srcPath.substr(found+1);

  // int srcSampRate = resamp.get_srcSampleRate();
  // int tgtSampleRate = resamp.get_tgtSampleRate();
  std::stringstream ss;
  ss << std::setw(4) << std::setfill('0') << srcSampleRate;  // Insert leading zero
  std::string srcSampleRateStr = ss.str();
  ss.str(std::string());
  ss << std::setw(4) << std::setfill('0') << tgtSampleRate;
  std::string tgtSampleRateStr = ss.str();

  std::string resampStr = srcSampleRateStr + "to" + tgtSampleRateStr + "ppi";

  found = fname.find_last_of( "." );
  std::string bname = fname.substr(0,found);
  std::string extension = fname.substr(found);   // pick up the . (dot)

  out = separator() + bname + "-" + resampStr + extension;

  return out;
}

/**
Read source image from disk. Source image must be 8-bit grayscale.
@return image matrix (2-dim) in space domain
*/
cv::Mat readImage( const std::string image_path )
{
  cv::Mat image = cv::imread( image_path, cv::COLOR_BGR2GRAY );
  return image;
}

/**
Generate list of source files to resample.  Build the glob-expression using the
image filename extension to generate the list.  If nothing is found, the
OUT list is empty.

@param dir that should contain imagery
@param fmt image compression format by filename extension
@param &v - OUT, alphanumeric, sorted list of file paths
*/
void retrieveSourceImagesList( std::string dir, std::string fmt, std::vector<std::string> &v )
{
  std::string globExpr =  dir + "/*." + fmt;
  glob::Glob glob(globExpr);
  std::string globFname{};
  while( glob ) {
    globFname = glob.GetFileName();
    if( (globFname == "." ) || (globFname == ".." ) )   // Req'd for windows.
    {
      glob.Next();
      continue;
    }
    v.push_back( dir + "/" + glob.GetFileName() );
    glob.Next();
  }

  std::sort( v.begin(), v.end() ); 
}
