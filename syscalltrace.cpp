#include <iostream>
#include <fstream>
#include <string>
#include "pin.H"
#include <sys/syscall.h>

#define RESOLVE_NAME_AND_PARAMS  1

using namespace std;

std::ofstream TraceFile;

#if RESOLVE_NAME_AND_PARAMS

typedef struct sysent {
        unsigned nargs;
        const char *sys_name;
} struct_sysent;

const struct_sysent sysinfo[] = {
  #include "syscallsinfo.h"
};

#endif

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "syscalls.out", "specify file name for syscalls profile ");

// Print syscall number and arguments
VOID SysBefore(THREADID threadIndex, ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5) {
  
  #if RESOLVE_NAME_AND_PARAMS //------------------------------------------------

    TraceFile << "[" << dec << PIN_GetPid() << "-" << threadIndex << "] " << hex << "0x" << (unsigned long)ip << ": " <<  sysinfo[(long)num].sys_name;
    switch(sysinfo[(long)num].nargs) {
      case 5: 
        TraceFile << hex  << "(0x" << (unsigned long)arg0 << ", 0x" << (unsigned long)arg1 << ", 0x" << (unsigned long)arg2 << ", 0x" <<  (unsigned long)arg3 << ", 0x" << (unsigned long)arg4 << ", 0x" << (unsigned long)arg5 << ") "<< endl;
      break;
      case 4:
        TraceFile << hex  << "(0x" << (unsigned long)arg0 << ", 0x" << (unsigned long)arg1 << ", 0x" << (unsigned long)arg2 << ", 0x" <<  (unsigned long)arg3 << ") "<< endl;
      break;
      case 3:
        TraceFile << hex  << "(0x" << (unsigned long)arg0 << ", 0x" << (unsigned long)arg1 << ", 0x" << (unsigned long)arg2 << ")"<< endl;
      break;
      case 2:
        TraceFile << hex  << "(0x" << (unsigned long)arg0 << ", 0x" << (unsigned long)arg1 << ")"<< endl;
      break;
      default: 
        TraceFile << hex  << "(0x" << (unsigned long)arg0 << ")"<< endl;
  }

  #else //----------------------------------------------------------------------

  TraceFile << "[" << dec << PIN_GetPid() << "-" << threadIndex << "] " << hex << "0x" << (unsigned long)ip << ": " << dec << (long)num;
  TraceFile << hex << "(0x" << (unsigned long)arg0 << ", 0x" << (unsigned long)arg1 << ", 0x" << (unsigned long)arg2 << ", 0x" <<  (unsigned long)arg3 << ", 0x" << (unsigned long)arg4 << ", 0x" << (unsigned long)arg5 << ")" << endl;

  #endif //---------------------------------------------------------------------

     
}

// Print the return value of the system call
VOID SysAfter(THREADID threadIndex, ADDRINT ret) {
  TraceFile << "[" << dec << PIN_GetPid() << "-" << threadIndex << "]="  << (unsigned long)ret << endl;
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v) {
  SysBefore(
    threadIndex,
    PIN_GetContextReg(ctxt, REG_INST_PTR),
    PIN_GetSyscallNumber(ctxt, std),
    PIN_GetSyscallArgument(ctxt, std, 0),
    PIN_GetSyscallArgument(ctxt, std, 1),
    PIN_GetSyscallArgument(ctxt, std, 2),
    PIN_GetSyscallArgument(ctxt, std, 3),
    PIN_GetSyscallArgument(ctxt, std, 4),
    PIN_GetSyscallArgument(ctxt, std, 5)
  );
}

VOID SyscallExit(THREADID threadIndex, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v) {
    SysAfter(threadIndex, PIN_GetSyscallReturn(ctxt, std));
}

BOOL signalHandler (THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v) {
    PIN_Detach();
    return 0;
}

// This function is called when the application exits
// It prints the name and count for each procedure
void DetachCallback(void *v) {
  TraceFile.flush();
  TraceFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

int Usage() {
    PIN_ERROR("This tool prints a log of system calls" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
 PIN_InitSymbols();
 
 if( PIN_Init(argc,argv) ) return Usage();

 string filename =  KnobOutputFile.Value();
 TraceFile.open(filename.c_str());


 if(TraceFile.fail()) {
    PIN_ERROR("Output file cant be opened");
    return -1;
 }

 if( !PIN_InterceptSignal(12, signalHandler, 0) ) {
    PIN_ERROR("PIN_InterceptSignal FAILED!");
    return -2;
 }

 PIN_AddSyscallEntryFunction(SyscallEntry, 0);
 PIN_AddSyscallExitFunction(SyscallExit, 0);

 // Register Fini to be called when the application exits
 PIN_AddDetachFunction(DetachCallback, 0);

 PIN_StartProgram();

 return 0;
}
