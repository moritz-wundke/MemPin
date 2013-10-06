/**
 * This file is part of the mempin project. A specialized pintool for memory tracking and
 * optimization.
 *
 * Copyright (c) 2012, Moritz Wundke
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the owner nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Moritz Wundke BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/** MemPin includes */
#include "mempin.h"

// 
// This extended Pin Tool performs the required actions for exercise 2.2
//

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "MemPin.out", "specify output file name. PID will be added. The file will be in CSV format where applicable.");

KNOB<INT32> KnobAnalysisTool(KNOB_MODE_WRITEONCE, "pintool",
    "tool", "0", "analysis tool to be used. See README for more information.");

INT32 gPinPid = 0;

// The outfile
ofstream OutFile;
PIN_LOCK OutFileLock;

const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

//
// Tool registration implementation
//

std::vector<tool_init> gToolRegister;

/** Register a tool */
void register_tool(tool_init tool)
{
    gToolRegister.push_back(tool);
}

/** Start the given tool with the given ID */
void start_tool(INT32 toolId)
{
    for(std::vector<tool_init>::iterator it = gToolRegister.begin(); it != gToolRegister.end(); ++it) {
        if ( (*it)(toolId) == TRUE )
            return;
    }
    ERROR("No tool found for id: " << toolId);
}


/** Add all our tools */
void register_tools()
{
    // Register our tools.
    // TODO: Tools should be compiled separatly and loaded as a shared lib
    register_tool(inscount);
    register_tool(inscount_ext);
    register_tool(proccount);
    register_tool(malloctrace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "MemPin - Memory and Instruction optimization tool" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    gPinPid = PIN_GetPid();

    //INT32 tool = KnobAnalysisTool.Value();
    LOGI("Starting mempin tool with tool: " << KnobAnalysisTool.Value());

    // Add the PID to the output file so that we have one for each process in MPI
    char *outFileName = new char[KnobOutputFile.Value().size()+10];
    sprintf(outFileName, "%s_%d", KnobOutputFile.Value().c_str(), gPinPid);

    OutFile.open(outFileName);

    // We need a lock for the output file
    InitLock(&OutFileLock);

    // Start PinTool
    register_tools();
    start_tool(KnobAnalysisTool.Value());

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
