# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "F:/Yicvot/Yicvot/external/cpp_taskflow-src"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-build"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/tmp"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src"
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
