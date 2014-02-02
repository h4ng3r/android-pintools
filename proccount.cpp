#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include "pin.H"

using namespace std;

std::ofstream TraceFile;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "proccount.out", "specify file name for proccount profile ");


typedef struct RtnCount {
    string _name;
    string _image;
    ADDRINT _address;
    UINT64 _rtnCount;
    struct RtnCount * _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

// This function is called before every instruction is executed
VOID docount(UINT64 * counter) {
    (*counter)++;
}
   
const char * StripPath(const char * path) {
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, void *v) {
    
    // Allocate a counter for this routine
    RTN_COUNT * rc = new RTN_COUNT;

    IMG image = SEC_Img(RTN_Sec(rtn));
    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name = RTN_Name(rtn);
    rc->_image = StripPath(IMG_Name(image).c_str());
    rc->_address = RTN_Address(rtn);
    rc->_rtnCount = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;
            
    RTN_Open(rtn);        
    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_rtnCount), IARG_END);
    RTN_Close(rtn);

}

BOOL signalHandler (THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v) {
    PIN_Detach();
    return 0;
}

// This function is called when the application exits
// It prints the name and count for each procedure
VOID DetachCallback(void *v) {

    std::map<std::string, int> ImageCount;
    std::map<std::string, int>::iterator it;
    double total_calls = 0.0;
    int total_functions = 0;


    for (RTN_COUNT * rc = RtnList; rc != NULL; rc = rc->_next) {
        if (rc->_rtnCount > 0) {
            it = ImageCount.find(rc->_image);
            if ( it == ImageCount.end()) ImageCount.insert ( std::pair<std::string, int>(rc->_image,rc->_rtnCount) );
            else  it->second += rc->_rtnCount;
            ++total_functions;
            total_calls += rc->_rtnCount;
        }
    }
   
   TraceFile << "Traced " << total_calls << " calls of " << total_functions << " functions provided by " << ImageCount.size() << " libraries." << endl;

   TraceFile << endl << endl;

   TraceFile << "Image,Invocations,%Invocations" << endl;
   for (it=ImageCount.begin(); it!=ImageCount.end(); ++it)
        TraceFile << it->first << "," << it->second << "," << setiosflags(ios::fixed) << setprecision(4) << (it->second/total_calls)*100.0  << endl;


    TraceFile << endl << endl;
    TraceFile << "Procedure,Image,Address,Calls" << endl;
    for (RTN_COUNT * rc2 = RtnList; rc2 != NULL; rc2 = rc2->_next)
    {
        if (rc2->_rtnCount > 0)
            TraceFile <<  rc2->_name << ","
                  << rc2->_image << ","
                  << hex << rc2->_address <<  ","
                  << dec << rc2->_rtnCount << endl;
    }

    TraceFile.flush();
    TraceFile.close();

}


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

int Usage()
{
    cerr << "This Pintool counts the number of times a routine is executed" << endl;
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
