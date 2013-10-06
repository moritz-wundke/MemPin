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
#include "mempin_proccount.h"

//
// Tool Registration
//

BOOL proccount(INT32 toolId)
{
    if ( toolId == TOOL_PROCCOUNT )
    {
    	LOGI("Registering callbacks for proccount");

    	// Register Routine to be called to instrument rtn
    	RTN_AddInstrumentFunction(Proccount_Instruction, 0);

	    // Register Fini to be called when the application exits.
	  	PIN_AddFiniFunction(Proccount_Fini, 0);
        return TRUE;
    }
    return FALSE;
}

//
// ProcCount implemention
//

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

// The count implementation
VOID PIN_FAST_ANALYSIS_CALL proccount_docount(UINT64 * counter)
{
	(*counter)++;
}

// For eacj instruction count what is in there
VOID Proccount_Instruction(RTN rtn, VOID *v)
{
	// Allocate a counter for this routine
	RTN_COUNT * rc = new RTN_COUNT;

	// The RTN goes away when the image is unloaded, so save it now
	// because we need it in the fini
	rc->_name = RTN_Name(rtn);
	rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
	rc->_address = RTN_Address(rtn);
	rc->_icount = 0;
	rc->_rtnCount = 0;

	// Add to list of routines
	rc->_next = RtnList;
	RtnList = rc;

	RTN_Open(rtn);

	// Insert a call at the entry point of a routine to increment the call count
	RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)proccount_docount, IARG_PTR, &(rc->_rtnCount), IARG_END);

	// For each instruction of the routine
	for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
	{
		// Insert a call to proccount_docount to increment the instruction counter for this rtn
		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)proccount_docount, IARG_PTR, &(rc->_icount), IARG_END);
	}


	RTN_Close(rtn);
}

// This function is called when the application exits
VOID Proccount_Fini(INT32 code, VOID *v)
{
	GetLock(&OutFileLock, BASE_LOCK_TAG);
	// Write all collected data to the output file
	OutFile << setw(23) << "Procedure" << " "
			<< setw(15) << "Image" << " "
			<< setw(18) << "Address" << " "
			<< setw(12) << "Calls" << " "
			<< setw(12) << "Instructions" << endl;

	for (RTN_COUNT * rc = RtnList; rc; rc = rc->_next)
	{
		if (rc->_icount > 0)
			OutFile << setw(23) << rc->_name << " "
					<< setw(15) << rc->_image << " "
					<< setw(18) << hex << rc->_address << dec <<" "
					<< setw(12) << rc->_rtnCount << " "
					<< setw(12) << rc->_icount << endl;
	}
    OutFile.close();
    ReleaseLock(&OutFileLock);
}