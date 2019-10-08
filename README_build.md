----------
## NFIR Overview
The NIST Fingerprint Image Resampler **NFIR** is available as a command-line/console application on both windows and Linux platforms.  Runtime configuration data are read from either a configuration file or typical command-line switches.

See README for more details.

## Build the App on Linux
The build as described was tested on CentOS 7 and Windows Subsystem for Linux (WSL) Ubuntu 18.04.

If your version of OpenCV was built and installed from source into default directories, then path(s) to the include files will already be available per `pkg-config` and not required to be specified on the `make` command line nor in the Makefile.

### OpenCV
To check if OpenCV include files are available, try the following command.  If you see something similar, you're probably ok.  If not, then add the path to OpenCV include files to the default paths used by gcc compiler and linker.

```
~$ pkg-config --cflags opencv
-I/usr/local/include/opencv -I/usr/local/include

~$ pkg-config --libs opencv
-L/usr/local/lib -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
```

Just in case, here is an example of the `opencv.pc` file generated when the OpenCV libs were built:

```
user@computer:/usr/local/lib/pkgconfig$ cat opencv.pc
# Package Information for pkg-config

prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir_old=${prefix}/include/opencv
includedir_new=${prefix}/include

Name: OpenCV
Description: Open Source Computer Vision Library
Version: 3.4.3
Libs: -L${exec_prefix}/lib -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
Libs.private: -ldl -lm -lpthread -lrt
Cflags: -I${includedir_old} -I${includedir_new}
```

### Run `make`
Run make as follows:
```
./NFIR$ cd build
./NFIR/build$ make -f Makefile.linux makedirs
./NFIR/build$ make -f Makefile.linux
```


# Check Build OK
Check `./NFIR/bin` and `./NFIR/lib/static` dir:

<pre><code>
./NFIR/bin$ ll nfir
 
-rwxrwxrwx 1 user user   791720 Sep 17 08:52 nfir*

./NFIR/lib/static$ ll nfir.lib

-rwxrwxrwx 1 user user 480958 Sep 17 08:52 libnfir.a*

./NFIR/bin$ ./nfir -v
NFIR (NIST Fingerprint Image Resampler) version: 0.1.0
OpenCV version: 3.4.3
</code></pre>


## Build the App on Windows
Visual Studio 2017 was used to build the **NFIR**.  Start here to download the installer:  https://visualstudio.microsoft.com/vs/older-downloads/.

After installation of the installer, run it to install the Visual C++ build tools including Windows 10 SDK, CMake, and Redistributable Update.  Once complete, open a `x64 Native Tools Command Prompt for VS 2017` window and navigate to the `build` directory under this distribution.

### OpenCV
For windows, it is not necessary to build the development and runtime libraries.  Navigate to https://opencv.org/releases/, click the desired version link to download the installer file, for example, `opencv-3.4.5-vc14_vc15.exe`.  Run the installer to save the libraries.

Paths to the OpenCV library are specified either on the command line or the Makefile.  The Makefile.win64 variable OPENCV_LIBS must be uncommented.  For example, if the OpenCV 2.4.x library files are installed in the `D:\OpenCV\opencv_24136` root, then uncomment the `OPENCV_LIBS=$(OPENCV_LIBSDIR)\opencv_core2413.lib $(OPENCV_LIBSDIR)\opencv_imgproc2413.lib $(OPENCV_LIBSDIR)\opencv_highgui2413.lib` variable in the Makefile and run the command line build as below:

```
.\NFIR\build> nmake OPENCV_INCDIR=D:\OpenCV\opencv_24136\build\include OPENCV_LIBSDIR=D:\OpenCV\opencv_24136\build\x64\vc14\lib -f Makefile.win64
```

If the OpenCV 3.4.x library files are installed in the `D:\OpenCV\opencv_345` root, then uncomment the `OPENCV_LIBS=$(OPENCV_LIBSDIR)\opencv_world345.lib` variable in the Makefile and run the command line build:

```
.\NFIR\build> nmake OPENCV_INCDIR=D:\OpenCV\opencv_345\build\include OPENCV_LIBSDIR=D:\OpenCV\opencv_345\build\x64\vc14\lib -f Makefile.win64
```

# Check Build OK
Check `.\NFIR\bin` and `.\NFIR\lib\static` dir:

<pre><code>
.\NFIR\bin> dir nfir.exe
 
   06/20/2019  01:01 PM           663,040 nfir.exe

.\NFIR\lib\static> dir nfir.lib

   06/20/2019  01:01 PM           978,094 nfir.lib

.\NFIR\bin> nfir.exe -v
NFIR (NIST Fingerprint Image Resampler) version: 0.1.0
OpenCV version: 3.4.5
</code></pre>

## Doc the App
Use `doxygen  (http://www.doxygen.nl/)` to generate HTML under the `doc` directory:
```
.\NFIR\doc> doxygen
```
