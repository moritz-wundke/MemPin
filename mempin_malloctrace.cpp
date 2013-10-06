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
#include "mempin_malloctrace.h"

//
// Tool Registration
//

BOOL malloctrace(INT32 toolId)
{
    if ( toolId == TOOL_MALLOCTRACE )
    {
    	LOGI("Registering callbacks for malloctrace");

    	// Register ImageLoad to be called when each image is loaded.
    	IMG_AddInstrumentFunction(MallocTrace_ImageLoad, 0);

    	// Register Analysis routines to be called when a thread begins/ends
	    PIN_AddThreadStartFunction(MallocTrace_ThreadStart, 0);
	    PIN_AddThreadFiniFunction(MallocTrace_ThreadFini, 0);

	    // Register Fini to be called when the application exits
	    PIN_AddFiniFunction(MallocTrace_Fini, 0);
        
        return TRUE;
    }
    return FALSE;
}

//
// Malloc Trace implemention
//
// Note that opening a file in a callback is only supported on Linux systems.

VOID MallocTrace_ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    GetLock(&OutFileLock, threadid+1);
    OutFile << "thread begin " << threadid << endl;
    ReleaseLock(&OutFileLock);
}

VOID MallocTrace_ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    GetLock(&OutFileLock, threadid+1);
    OutFile << "thread " << threadid << " end code(" << code << ")" << endl;
    ReleaseLock(&OutFileLock);
}

VOID MallocTrace_BeforeMalloc( int size, THREADID threadid )
{
    GetLock(&OutFileLock, threadid+1);
    OutFile << "thread " << threadid << " entered malloc(" << size << ")" << endl;
    ReleaseLock(&OutFileLock);
}

VOID MallocTrace_AfterMalloc(ADDRINT ret, THREADID threadid)
{
    GetLock(&OutFileLock, threadid+1);
    OutFile << "thread " << threadid << " after malloc ret(" << ret << ")" << endl;
    ReleaseLock(&OutFileLock);
}

VOID MallocTrace_ImageLoad(IMG img, VOID *)
{
	RTN rtn = RTN_FindByName(img, "malloc");

    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(MallocTrace_BeforeMalloc),
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_THREAD_ID, IARG_END);
		RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(MallocTrace_AfterMalloc),
			IARG_FUNCRET_EXITPOINT_VALUE,
			IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);
    }
}

// This routine is executed once at the end.
VOID MallocTrace_Fini(INT32 code, VOID *v)
{
	GetLock(&OutFileLock, BASE_LOCK_TAG);
    OutFile.close();
    ReleaseLock(&OutFileLock);
}