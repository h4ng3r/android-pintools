android-pintools
================

This are my own pintools made for a college project.

All of them are shared here with a GPL 2 licence, that means that they can be changed or used in any manner you want. But if you know how to improve any of them, please notify me.


# Installation

1. Download the pintools inside %PIN-DIR%/source/tools
git clone https://github.com/h4ng3r/android-pintools.git

2. Download Android NDK 9

3. Install the buildchange

# Compilation

To compile the decode.cpp use standard g++ decode.cpp -o decode.
For the pintools use: 

  make  HOST_ARCH=ia32 TARGET_OS=android CC=%TOOL_CHAIN_PATH%/bin/gcc CXX=/%TOOL_CHAIN_PATH%/bin/g++ obj-ia32/%TOOL%.so
  
# Execute

To execute any pintool use:

  %PACKAGE% var is instrumented APK package (i.e. com.android.mms)
  
  %TOOL% var is the pintool name
  
  The output can be also set to /sdcard/
  
  OUTPUT="/data/data/%PACKAGE%/%TOOL%.out"
  
  adb shell su - /data/pin/pin -pid %PID% -logfile /data/data/%PACKAGE%/log.out -t /data/pin/obj-ia32/%TOOL%.so -o /data/data/%PACKAGE%/%TOO%L.out -logfile /data/data/%PACKAGE%/%TOOL%.log
