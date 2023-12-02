# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/mhkov/esp/esp-idf/components/bootloader/subproject"
  "C:/SO/touch_pad_interrupt/build/bootloader"
  "C:/SO/touch_pad_interrupt/build/bootloader-prefix"
  "C:/SO/touch_pad_interrupt/build/bootloader-prefix/tmp"
  "C:/SO/touch_pad_interrupt/build/bootloader-prefix/src/bootloader-stamp"
  "C:/SO/touch_pad_interrupt/build/bootloader-prefix/src"
  "C:/SO/touch_pad_interrupt/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/SO/touch_pad_interrupt/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/SO/touch_pad_interrupt/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
