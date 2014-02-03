android-pintools
================

This are my own pintools made for a college project.

All of them are shared here with a GPL 2 licence, that means that they can be changed or used in any manner you want. But if you know how to improve any of them, please notify me.


## Pintools

### Proccount

This pintool using RTN instrumentation creates a list with all executed routines and a call count for each. After recives USR2 signal (12) it prints the results in the output file and deatach. The output file starts with a simple summary, then a table with Image,Invocations,%Invocations and ends with Procedure,Image,Address,Calls.

### Proctrace

This pintool uses RTN instrumentation. It creates a list with all routines like proccount. But it assigns an identifier for each routine. The output is buffered and finished when recives USR2 signal. The output format is in binary starting with all identifiers, then char 0xFF and then all the list is writed in order. The routine list format is: name,image,address;. The decode.cpp can be used to prettify the output file.

### Syscallcount

This pintool uses PIN_AddSyscallEntryFunction to count all syscall calls. The number of calls are stored in a integer array. After USR2 signal, the pintool deatach and print the results in the format syscall,calls. 

### Syscalltrace

This pintool uses PIN_AddSyscallEntryFunction and PIN_AddSyscallExitFunction. For each syscall entry it prints the [pid-thread] instruction : syscallnum ( arg0, arg1, arg2, arg3, arg4, arg5). For each syscall exit it prints [pid-thread]=result. If the RESOLVE_NAME_AND_PARAMS is set to 1 in the compilation time the entry output is changed, the syscallnum become syscallname and only print the necessary params.

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

## TODO

1. Binary output for all pintools (only integers)
2. Binary output for strings
3. Decode.cpp for all new formats
4. Push the tester APK and some script

