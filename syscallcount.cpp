#include <iostream>
#include <fstream>
#include <string>
#include "pin.H"

#include <sys/syscall.h>

using namespace std;

std::ofstream TraceFile;

UINT64 syscount [444]; 

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "syscalls.out", "specify file name for syscalls profile ");

// Print syscall number and arguments
VOID SysBefore( ADDRINT num) {
       syscount[(int)num]++;
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v) {
    SysBefore(  PIN_GetSyscallNumber(ctxt, std) );
}


BOOL signalHandler (THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v) {
    PIN_Detach();
    return 0;
}

// This function is called when the application exits
// It prints the name and count for each procedure
void DetachCallback(void *v) {
  TraceFile << "SYSCALL NUM;CALL TIMES" << endl;
  for(int i = 0; i < 444; ++i)  {
    if(syscount[i] > 0) TraceFile << i << ";" << syscount[i] << endl;
  }
  TraceFile.flush();
  TraceFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

int Usage() {
    PIN_ERROR("This tool prints a count of all system calls \n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
 //PIN_InitSymbols();
 
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

 // Register Fini to be called when the application exits
 PIN_AddDetachFunction(DetachCallback, 0);

 PIN_StartProgram();

 return 0;
}
