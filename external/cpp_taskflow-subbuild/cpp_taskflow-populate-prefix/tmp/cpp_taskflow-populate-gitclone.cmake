# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitclone-lastrun.txt" AND EXISTS "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitinfo.txt" AND
  "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitclone-lastrun.txt" IS_NEWER_THAN "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitinfo.txt")
  message(STATUS
    "Avoiding repeated git clone, stamp file is up to date: "
    "'F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitclone-lastrun.txt'"
  )
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "F:/Yicvot/Yicvot/external/cpp_taskflow-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: 'F:/Yicvot/Yicvot/external/cpp_taskflow-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "D:/Software/Git/Git/cmd/git.exe" 
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/taskflow/taskflow.git" "cpp_taskflow-src"
    WORKING_DIRECTORY "F:/Yicvot/Yicvot/external"
    RESULT_VARIABLE error_code
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/taskflow/taskflow.git'")
endif()

execute_process(
  COMMAND "D:/Software/Git/Git/cmd/git.exe" 
          checkout "v3.6.0" --
  WORKING_DIRECTORY "F:/Yicvot/Yicvot/external/cpp_taskflow-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v3.6.0'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "D:/Software/Git/Git/cmd/git.exe" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "F:/Yicvot/Yicvot/external/cpp_taskflow-src"
    RESULT_VARIABLE error_code
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: 'F:/Yicvot/Yicvot/external/cpp_taskflow-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitinfo.txt" "F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: 'F:/Yicvot/Yicvot/external/cpp_taskflow-subbuild/cpp_taskflow-populate-prefix/src/cpp_taskflow-populate-stamp/cpp_taskflow-populate-gitclone-lastrun.txt'")
endif()
