# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/ebert/esp/esp-idf/components/bootloader/subproject"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix/tmp"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix/src/bootloader-stamp"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix/src"
  "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/GITHUB/SCRAPPY-MK1/src/SCRAPPY_CALIBRATION/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
