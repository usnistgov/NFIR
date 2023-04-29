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
#ifdef _WIN32
  #pragma warning(disable : 4996)  // 'ctime': This function or variable may be unsafe. Consider using ctime_s instead.
#endif

#include "CLI11.hpp"
#include "glob.h"
#include "nfir_lib.h"
#include "termcolor.h"


#include <chrono>
#include <ctime>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>


/** Well known sample rate of source image */
static int srcSampleRate{0};
/** Desired sample rate of generated, target image */
static int tgtSampleRate{0};
/** for the source image */
static std::string srcPath{""};
/** for the target (generated) image */
static std::string tgtPath{""};

static std::string buildTargetImageFilename( const std::string, const std::string );
static void retrieveSourceImagesList( const std::string, const std::string &, std::vector<std::string>& );

/**
 * @brief OS dependent path delimiter.
 * 
 * @return char '/' or '\'
 */
inline char separator()
{
#ifdef _WIN32_64
  return '\\';
#else
  return '/';
#endif
}


/**
 * @brief `main' is a special function in all C++ programs; it is the function
 * called when the program is run; execution for all C++ programs begins with
 * the main function regardless of where the function is actually located
 * within the code.
 *
 * Process the command-line or config file runtime parameters.  Based on the
 * source and target sample rates, run either up- or down-sample.
 *
 * For downsample, each image lowpass filter is unique.
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
  
  std::vector<std::string> vecPngTextChunk{""};
  app.add_option( "-e, --png-text-chunk", vecPngTextChunk, "list of 'tEXt' chunks in format 'keyword:text'" );

  std::string srcDir {};
  app.add_option( "-s, --src-dir", srcDir, "Source imagery dir (absolute or relative)" )
    ->check(CLI::ExistingDirectory);
  std::string tgtDir {};
  app.add_option( "-t, --tgt-dir", tgtDir, "Target imagery dir (absolute or relative)" )
    ->check(CLI::ExistingDirectory);

  std::string srcImageFormat {"png"};
  app.add_option( "-m, --src-img-fmt", srcImageFormat, "Image compression format by filename extension, default is 'png'" );

  std::string tgtImageFormat {"png"};
  app.add_option( "-n, --tgt-img-fmt", tgtImageFormat, "Image compression format by filename extension, default is 'png'" );

  std::string interpolationMethod {};
  CLI::Option *im_opt = app.add_option( "-i, --interp-method", interpolationMethod, "For interpolation use [ bicubic | bilinear ]" );

  std::string filterShape {};
  CLI::Option *fs_opt = app.add_option( "-f, --downsamp-filter-shape", filterShape, "For filter use [ ideal | gaussian ]" )
    ->needs(im_opt);
  im_opt->needs(fs_opt);

  bool flagDryRun {false};
  app.add_flag( "-x,--dry-run", flagDryRun, "Skip resample attempt" )
    ->multi_option_policy()
    ->ignore_case();

  bool flagVerify {true};
  app.add_flag( "-y,--verify", flagVerify, "Print config params prior to resampling; prompt to continue" )
    ->multi_option_policy()
    ->ignore_case();
    // ->take_last(false);  // DOES NOT COMPILE; only allowed once on cmd line

  bool flagVerbose {false};
  app.add_flag( "-z,--verbose", flagVerbose, "Print image paths and total runtime count" )
    ->multi_option_policy()
    ->ignore_case();

  bool flagVersion {false};
  app.add_flag( "-v,--version", flagVersion, "Print NFIR, OpenCV versions and exit" )
    ->multi_option_policy()
    ->ignore_case();

  bool flagPrintConfig {false};
  app.add_flag( "-p,--print-config", flagPrintConfig, "Print config file and exit" )
    ->multi_option_policy()
    ->needs(cf_opt)
    ->ignore_case();

  app.get_formatter()->column_width(20);

  // If cmd line has zero switches, force -h.
  if( argc==1 )
  {
    char hlp[8] = "--help\0";
    argv[argc++] = hlp;
    // Line below prevents CLI11 error: 'The following argument was not expected:'
    // when binary is run with zero params, ie, argc=1.
    std::cout << std::endl;
  }

  // This is what the macro CLI11_PARSE expands to:
  // CLI11_PARSE(app, argc, argv);
  try
  {
    app.parse( argc, argv );
  }
  catch( const CLI::ParseError &e )   // also catches help switch
  {
    app.exit(e);    // prints help menu to console
    return -1;
  }


  if( flagVersion )
  {
    std::cout << NFIR::printVersion() << std::endl;
    return(0);
  }

  if( flagPrintConfig )
  {
    std::cout << app.config_to_str(false, true) << std::endl;
    return(0);
  }

  // Output config data to console and prompt to continue.
  if( flagVerify )
  {
    std::cout << NFIR::printVersion() << std::endl;
    std::cout << "\n  *** Verify runtime parameters ***" << std::endl;
    std::cout << "Source sample rate: '" << srcSampleRate << "'" << std::endl;
    std::cout << "Target sample rate: '" << tgtSampleRate << "'" << std::endl;
    if( !srcFile.empty() ) {
      std::cout << "Source image file: '"  << srcFile << "'" << std::endl;
      std::cout << "Target image file: '"  << tgtFile << "'" << std::endl;
    }
    if( !srcDir.empty() ) {
      std::cout << "Source imagery dir: '" << srcDir  << "'" << std::endl;
      std::cout << "Target imagery dir: '" << tgtDir  << "'" << std::endl;
    }
    std::cout << "Source image format: '" << srcImageFormat  << "'" << std::endl;
    std::cout << "Target image format: '" << tgtImageFormat  << "'" << std::endl;
    if( tgtSampleRate < srcSampleRate ) {
      std::cout << "Downsample filter shape: '" << filterShape << "'" << std::endl;
      std::cout << "Downsample interpolation method: '" << interpolationMethod
                << "'" << std::endl;
    }
    else
    {
      std::cout << "Upsample interpolation method: '" << interpolationMethod
                << "'" << std::endl;
    }
    std::cout << "Dry-run: " << std::boolalpha << flagDryRun << std::endl;
    std::cout << "Verbose mode: " << std::boolalpha << flagVerbose << std::endl;

    char key_press{};
    bool loooop{ true };
    std::cout << "\nPress y to continue, n to exit:  ";
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
    retrieveSourceImagesList( srcDir, srcImageFormat, listSrcImages );
    if( listSrcImages.empty() ) {
      std::cout << "Source images dir empty: " << srcDir << std::endl;
      return 0;
    }
  }

  auto startStamp = std::chrono::system_clock::now();

  // START LOOP through all src images.
  for( auto it:listSrcImages )
  {
    if( srcFile != "" ) {   // source image specific by name in config
      tgtPath = tgtFile;
    }
    else {                  // source image(s) specified by dir in config
      std::string tgtFname = buildTargetImageFilename( it, tgtImageFormat );
      tgtPath = tgtDir + tgtFname;
    }

    // Load file into memory (block); get its length
    char *srcFileMemBlock;
    std::ifstream ifsSrcFile( it, std::ios::binary );
    ifsSrcFile.seekg( 0, std::ios::end );
    uint64_t lenSrcFileBlock = ifsSrcFile.tellg();
    ifsSrcFile.seekg( 0, std::ios::beg );
    srcFileMemBlock = new char[lenSrcFileBlock];
    ifsSrcFile.read( srcFileMemBlock, lenSrcFileBlock );
    ifsSrcFile.close();

    srcPath = it;
    std::cout << termcolor::blue << "src image: " << srcPath << termcolor::grey << std::endl;

    // Init NFIR resampler params.
    uint8_t* srcImageAry{NULL};       // source image data
    uint8_t* tmpImg{NULL};            // resampled image data
    uint8_t** tgtImageAry{&tmpImg};   // pointer to resampled image data
    uint32_t imageWidth{0};           // source IN and target OUT
    uint32_t imageHeight{0};          // source IN and target OUT
    std::vector<std::string>logRuntime;  // container for all log messages


    // Copy the file stream into memory for resampler call.
    srcImageAry = new uint8_t[lenSrcFileBlock];
    std::memcpy( srcImageAry, srcFileMemBlock, lenSrcFileBlock );


    if( !flagDryRun )
    {
      try {
        NFIR::resample( srcImageAry, tgtImageAry,
                        srcSampleRate, tgtSampleRate, "inch",
                        interpolationMethod, filterShape,
                        &imageWidth, &imageHeight, &lenSrcFileBlock,
                        srcImageFormat, tgtImageFormat, vecPngTextChunk,
                        logRuntime );

        std::ofstream outFile( tgtPath, std::ios::out | std::ios::binary );
        if( outFile.is_open() )
        {
          outFile.write( reinterpret_cast<char*>(*tgtImageAry), lenSrcFileBlock );
          outFile.close();
        }
        else
        {
          throw NFIR::Miscue( "Cannot open file for write: " + tgtPath );
        }
        tmp_count += 1;

        // {
        //   // Access for the intermediate, filtered image prior to downsample.
        //   // Uncomment this scope/section and set the tgtPath appropriately.
        //   NFIR::get_filteredImage( tgtImageAry, srcImageFormat, &lenSrcFileBlock, &imageWidth, &imageHeight );
        //   std::cout << "intermediate image Width:  " << imageWidth << std::endl;
        //   std::cout << "intermediate image Height: " << imageHeight << std::endl;
        //   std::cout << "intermediate image vector length: " << lenSrcFileBlock << std::endl;
        //   // std::string filteredPath{""};
        //   std::ofstream outFileIntermediateImage( filteredPath, std::ios::out | std::ios::binary );
        //   if( outFileIntermediateImage.is_open() )
        //   {
        //     outFileIntermediateImage.write( reinterpret_cast<char*>(*tgtImageAry), lenSrcFileBlock );
        //     outFile.close();
        //   }
        //   else
        //   {
        //     throw NFIR::Miscue( "Cannot open file for write: " + tgtPath );
        //   }
        // }

      }
      catch( const NFIR::Miscue &e ) {
        std::cout << termcolor::red << e.what() << std::endl;
        std::cout << "NFIR runtime log prior-to this exception:" << std::endl;
        for( auto s : logRuntime ) { std::cout << s << std::endl; }
        std::cout << termcolor::grey;
        return -1;
      }
    }

    if( flagVerbose )
    {
      if( flagDryRun )
      {
        tmp_count += 1;
        std::cout << "dry-run srcPath: " << srcPath << std::endl;
        std::cout << "dry-run tgtPath: " << tgtPath << std::endl;
        std::cout << "dry-run count: " << tmp_count << " of " << listSrcImages.size() << std::endl;
      }
      else
      {
        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "srcPath: " << srcPath << std::endl;
        std::cout << "tgtPath: " << tgtPath << std::endl;
        for( auto s : logRuntime ) { std::cout << s << std::endl; }
        std::cout << "RESAMPLE complete: " << tmp_count << " of " << listSrcImages.size() << std::endl;
      }
    }

  }   // END LOOP through all src images.

  std::chrono::system_clock::time_point endStamp = std::chrono::system_clock::now();
  std::time_t endTime = std::chrono::system_clock::to_time_t( endStamp );

  std::chrono::duration<double> elapsedSeconds{ endStamp-startStamp };

  std::cout << "Total RESAMPLED images count: " << tmp_count << std::endl;
  std::cout << "Finished computation: " << std::ctime(&endTime)
            << "Elapsed time: " << elapsedSeconds.count() << "s\n";
  return 0;
}


/**
 * @brief Supports glob operation for batch processing.
 * 
 * The target filename is based purely on the source filename. The only "change"
 * that is made is that the source sample rate is replaced by:
 *   "SRCRATEtoTGTRATE".
 *
 * EXAMPLE:
 *
 * src: A0001_10P_E01_300PPI.jpg
 *
 * tgt: A0001_10P_E01_0300to0500PPI.png  <-- note (allowable/configurable) change in extension
 *
 * @param srcPath path of source image
 * @param fmt target-image filename extension
 * @return filename of target image
 */
std::string buildTargetImageFilename( const std::string srcPath, const std::string fmt )
{
  std::string out{""};  // the string to be built to be returned

  // Prep the regex search expression per the source sample rate (via config param).
  // Just for case that source sample rate *might* be preceeded by zero, one, or more
  // leading zeroes, use reg-exp search to find the substring that will be replaced
  // by the "FROMtoTO" string (local variable resampStr).
  std::string rxStr{ "\\d*" };
  rxStr.append( std::to_string( srcSampleRate ) );
  rxStr.append( "ppi");   // sample rate MUST be followed immediately by [PPI | ppi].
  std::regex rx( rxStr, std::regex_constants::ECMAScript | std::regex_constants::icase );
  std::smatch matchResult;

  // Parse the base filename of the source from the full PATH.
  size_t found;
  found = srcPath.find_last_of( "/\\" );
  std::string folder = srcPath.substr(0,found);
  std::string tmp = srcPath.substr(found+1);
  found = tmp.find_last_of( "." );
  std::string bname = tmp.substr(0,found);

  // Prepend leading zeroes, if required, to the sample rates so that they
  // contain 4 chars.
  std::stringstream ss2;
  ss2 << std::setw(4) << std::setfill('0') << srcSampleRate;  // Insert leading zero
  std::string srcSampleRateStr = ss2.str();
  ss2.str(std::string());   // clear stream
  ss2 << std::setw(4) << std::setfill('0') << tgtSampleRate;
  std::string tgtSampleRateStr = ss2.str();
  // Build the "from-to" string.
  std::string resampStr = srcSampleRateStr + "to" + tgtSampleRateStr + "PPI";

  // Search for the source sample rate and replace with the "from-to" string.
  std::regex_search( bname, matchResult, rx );
  bname.replace( bname.find( matchResult[0] ), matchResult[0].length(), resampStr );

  out = separator() + bname + "." + fmt;

  return out;
}


/**
 * @brief Generate list of source files to resample.
 *
 * Build the glob-expression using the image filename extension to generate
 * the list.  If nothing is found, the OUT list is empty.
 *
 * @param dir that should contain imagery
 * @param fmt image compression format by filename extension
 * @param v   OUT, alphanumeric, sorted list of file paths
 */
void retrieveSourceImagesList( const std::string dir, const std::string &fmt, std::vector<std::string> &v )
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
    v.push_back( dir + separator() + glob.GetFileName() );
    glob.Next();
  }

  std::sort( v.begin(), v.end() ); 
}
