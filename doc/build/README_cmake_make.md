<link rel="stylesheet" href="doc/nfir.css" type="text/css" />

## Build NFIR on Windows
Visual Studio 2017 and 2019 may be used to build the **NFIR**.  Start here to download the installer:  https://visualstudio.microsoft.com/vs/older-downloads/.

After installation of the installer, run it to install the Visual C++ build tools including Windows 10 SDK, CMake, and Redistributable Update.  Open a `x64 Native Tools Command Prompt for VS 2019` window and navigate to the `build` directory under this distribution.  The same two commands work in Powershell.

```
**********************************************************************
** Visual Studio 2019 Developer Command Prompt v16.10.3
** Copyright (c) 2021 Microsoft Corporation
**********************************************************************

1) X:\.\NFIR\build> cmake -S .. -B R:\NFIR -DCMAKE_CONFIGURATION_TYPES="Release" -D_WIN32_64=1

2) X:\.\NFIR\build> MSBuild R:\NFIR\NFIR_ITL.sln
```

The location of the built library and executable are here:

```
R:\NFIR\src\lib\Release\NFIR_ITL.lib
R:\NFIR\src\bin\Release\NFIR_bin.exe
```

### OpenCV
For windows, it is not necessary to build the development and runtime libraries.  Navigate to https://opencv.org/releases/, click the desired version link to download the installer file, for example, `opencv-4.4.0-vc14_vc15.exe`.  Run the installer to extract the development and runtime libraries. (Optional: rename the extracted `opencv` root dir to one reflecting the OpenCV version, eg, `opencv-4.4.0`).

## Build NFIR on Linux

```
user@server:~/./NFIR/build$ cmake ..

sovia@server:~/./NFIR/build$ make
Scanning dependencies of target NFIR
[  9%] Building CXX object src/lib/CMakeFiles/NFIR.dir/filter_mask.cpp.o
[ 18%] Building CXX object src/lib/CMakeFiles/NFIR.dir/filter_mask_gaussian.cpp.o
[ 27%] Building CXX object src/lib/CMakeFiles/NFIR.dir/filter_mask_ideal.cpp.o
[ 36%] Building CXX object src/lib/CMakeFiles/NFIR.dir/nfir_lib.cpp.o
[ 45%] Building CXX object src/lib/CMakeFiles/NFIR.dir/resample.cpp.o
[ 54%] Building CXX object src/lib/CMakeFiles/NFIR.dir/resample_down.cpp.o
[ 63%] Building CXX object src/lib/CMakeFiles/NFIR.dir/resample_up.cpp.o
[ 72%] Linking CXX static library libNFIR.a
[ 72%] Built target NFIR
Scanning dependencies of target NFIR_bin
[ 81%] Building CXX object src/bin/CMakeFiles/NFIR_bin.dir/nfir.cpp.o
[ 90%] Building CXX object src/bin/CMakeFiles/NFIR_bin.dir/__/__/thirdparty/szx/glob/glob_posix.cpp.o
[100%] Linking CXX executable NFIR_bin
[100%] Built target NFIR_bin
```

The location of the built library and executable are here:

```
./NFIR/build/src/lib/libNFIR_ITL.a
./NFIR/build/src/bin/NFIR_bin
```

### OpenCV
To check if OpenCV include files are available, try the following command.  If you see something similar, you're probably ok.  If not, then add the path to OpenCV include files to the default paths used by gcc compiler and linker.

```
$ pkg-config --cflags opencv
-I/usr/local/include/opencv -I/usr/local/include

$ pkg-config --libs opencv
-L/usr/local/lib -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
```

Here is an example of the `opencv.pc` file generated when the OpenCV libs were built:

```
$ cat /usr/local/lib/pkgconfig/opencv.pc
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

Since OpenCV versions 3.4 and 4.5 were tested, they were installed *side by side* under the `/usr/home dir`.
By installing here instead of the default bins and libs dirs, more flexibility is enabled.

*It is not required to install more than one version of OpenCV; nor is it required that OpenCV is installed under
the user's home dir.*

Unlike Windows, OpenCV is built from source.  For any Linux version, click on the `opencv.org/releases/` Sources link to
download.  The location of the downloaded archive, the extraction, and the subsequent build, should be a directory,
for example, `~/Downloads/OpenCV`, other than the one where the installation occurs.
