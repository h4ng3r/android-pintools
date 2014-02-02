#include <iostream>
#include <fstream>
#include <string>
#include "pin.H"

using namespace std;

std::ofstream TraceFile;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "proctrace2.out", "specify file name for proctrace2 profile ");

int ident = 0;
char* BUFFER = new char[400];
int buff_count = 0;

typedef struct RtnCount {
    string _name;
    string _image;
    ADDRINT _address;
    struct RtnCount * _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

VOID trace(ADDRINT ident) {   
    BUFFER[buff_count] = ((0xff000000 & ident) >> 24);
    BUFFER[buff_count+1] = ((0x00ff0000 & ident) >> 16);
    BUFFER[buff_count+2] = ((0x0000ff00 & ident) >> 8);
    BUFFER[buff_count+3] = (0x000000ff & ident);

    buff_count += 4;
    if( buff_count == 400 ) {
        TraceFile.write(BUFFER, 400);
        buff_count = 0;
    }
}

const char * StripPath(const char * path) {
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, VOID *v) {

     RTN_COUNT * rc = new RTN_COUNT;

    IMG image = SEC_Img(RTN_Sec(rtn));
    rc->_name = RTN_Name(rtn);
    rc->_image = StripPath(IMG_Name(image).c_str());
    rc->_address = RTN_Address(rtn);
    
    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;

    ident++; // Identifies all procedure calls using an integer

    RTN_Open(rtn);    
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)trace, IARG_ADDRINT, ident, IARG_END);
    RTN_Close(rtn);

}

BOOL signalHandler (THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v) {
    PIN_Detach();
    return 0;
}

// This function is called when the application exits
// It prints the name and count for each procedure
VOID DetachCallback(VOID *v) {
    if( buff_count > 0 ) TraceFile.write(BUFFER, buff_count);

    TraceFile << char(0xFF);
    for (RTN_COUNT * rc2 = RtnList; rc2 != NULL; rc2 = rc2->_next) {
            TraceFile <<  rc2->_name << "," << rc2->_image << "," << hex << rc2->_address  << ";";
    }

    TraceFile.flush();
    TraceFile.close();
}


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

int Usage() {
    cerr << "This Pintool trace al executed routines in binary format" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
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

 // Register Instrumentation Function
 RTN_AddInstrumentFunction(Routine, 0);

 // Register Fini to be called when the application exits
 PIN_AddDetachFunction(DetachCallback, 0);

 PIN_StartProgram();

 return 0;
}
