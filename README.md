android-pintools
================

This are my own pintools made for a college project.

All of them are shared here with a GPL 2 licence, that means that they can be changed or used in any manner you want. But if you know how to improve any of them, please notify me.


## Installation

1. Download the pintools inside %PIN-DIR%/source/tools
2. Download Android NDK 9
3. Install the buildchange

## Compilation

To compile the decode.cpp use standard g++ decode.cpp -o decode.
For the pintools use: 
``` bash
  # $TOOL_CHAIN_PATH => path to your tool chain path (i.e. /usr/android)
  # $TOOL => pintool name without cpp
  
  make  HOST_ARCH=ia32 TARGET_OS=android CC=$TOOL_CHAIN_PATH/bin/gcc CXX=/$TOOL_CHAIN_PATH/bin/g++ obj-ia32/$TOOL.so
```

## Execute

To execute any pintool use:
``` bash
  # $PID => the instrumented APK process identifier
  # $PACKAGE => the instrumented APK package (i.e. com.android.mms)
  # $TOOL => pintool name without cpp
  # The output can be also set to /sdcard/
  
  adb shell su - /data/pin/pin -pid $PID -logfile /data/data/$PACKAGE/log.out -t /data/pin/obj-ia32/$TOOL.so -o /data/data/$PACKAGE/$TOOL.out -logfile /data/data/$PACKAGE/$TOOL.log
  adb pull /data/data/$PACKAGE/$TOOL.out # collect data
```
