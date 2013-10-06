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
#include "mempin_inscount.h"

// Lock used for thread count
PIN_LOCK lock;
INT32 numThreads = 0;

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key;

//
// Tool Registration
//

BOOL inscount(INT32 toolId)
{
    if ( toolId == TOOL_INSCOUNT )
    {
    	LOGI("Registering callbacks for inscount");

    	// Initialize the lock
        InitLock(&lock);

        // Obtain  a key for TLS storage.
        tls_key = PIN_CreateThreadDataKey(0);

    	// Callback for thread creation
    	PIN_AddThreadStartFunction(Inscount_ThreadStart, 0);

        // Register the trace callback
	    TRACE_AddInstrumentFunction(Inscount_Trace, 0);

	    // Register Fini to be called when the application exits.
	    PIN_AddFiniFunction(Inscount_Fini, 0);
        return TRUE;
    }
    return FALSE;
}

BOOL inscount_ext(INT32 toolId)
{
    if ( toolId == TOOL_INSCOUNT_EXT )
    {
    	LOGI("Registering callbacks for extended inscount");

    	// Initialize the lock
        InitLock(&lock);

        // Obtain  a key for TLS storage.
        tls_key = PIN_CreateThreadDataKey(0);

	    // Callback for thread creation
    	PIN_AddThreadStartFunction(Inscount_ThreadStart, 0);

        // Register the trace callback
	    TRACE_AddInstrumentFunction(Inscount_Trace, 0);

	    // Register the instruction callback
    	INS_AddInstrumentFunction(Inscount_Ext_Instruction, 0);

	    // Register Fini to be called when the application exits.
	    PIN_AddFiniFunction(Inscount_Ext_Fini, 0);
        return TRUE;
    }
    return FALSE;
}

//
// Common implementation
//

// function to access thread-specific data
thread_data_t* get_tls(THREADID threadid)
{
    thread_data_t* tdata = 
          static_cast<thread_data_t*>(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}

//
// Inscount Base implemention
//

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL inscount_docount(UINT32 c, THREADID threadid)
{
    thread_data_t* tdata = get_tls(threadid);
    tdata->_count += c;
}

VOID Inscount_ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    GetLock(&lock, threadid+1);
    numThreads++;
    ReleaseLock(&lock);

    thread_data_t* tdata = new thread_data_t;

    PIN_SetThreadData(tls_key, tdata, threadid);
}

// Pin calls this function every time a new basic block is encountered.
// It inserts a call to docount.
VOID Inscount_Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount for every bbl, passing the number of instructions.
        
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)inscount_docount, IARG_FAST_ANALYSIS_CALL,
                       IARG_UINT32, BBL_NumIns(bbl), IARG_THREAD_ID, IARG_END);
    }
}

// This function is called when the application exits
VOID Inscount_Fini(INT32 code, VOID *v)
{
    GetLock(&OutFileLock, BASE_LOCK_TAG);
    // Write to a file since cout and cerr maybe closed by the application
    OutFile << "Id,Instructions" << endl;
    
    for (INT32 t=0; t<numThreads; t++)
    {
        thread_data_t* tdata = get_tls(t);
        OutFile << decstr(t) << "," << tdata->_count << endl;
    }

    OutFile.close();
    ReleaseLock(&OutFileLock);
}

//
// Inscount Extended implementation
//

// Record memory reads for this thread
VOID RecordMemRead(THREADID threadid)
{
    thread_data_t* tdata = get_tls(threadid);
    tdata->_reads++;
}

// Record memory writes for this thread
VOID RecordMemWrite(THREADID threadid)
{
    thread_data_t* tdata = get_tls(threadid);
    tdata->_writes++;
}

VOID BranchCount( INT32 taken, THREADID threadid)
{
    if( !taken ) return;
    thread_data_t* tdata = get_tls(threadid);
    tdata->_branches++;
}

VOID FloatOpsCount(THREADID threadid)
{
    thread_data_t* tdata = get_tls(threadid);
    tdata->_floatOps++;
}

VOID Inscount_Ext_Instruction(INS ins, void *v)
{
    // Count the number of branches
    if (INS_IsDirectBranchOrCall(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) BranchCount, 
                       IARG_BRANCH_TAKEN,
                       IARG_THREAD_ID,
                       IARG_END);          
    }
    
    // Count the number of predicted instructions (float point, not 100% accurate!)
    else if ( INS_IsPredicated(ins) )
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)FloatOpsCount, IARG_THREAD_ID, IARG_END); 
    }

    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // The IA-64 architecture has explicitly predicated instructions. 
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead, IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, IARG_THREAD_ID,
                IARG_END);
        }
    }
}

// This function is called when the application exits
VOID Inscount_Ext_Fini(INT32 code, VOID *v)
{
    GetLock(&OutFileLock, BASE_LOCK_TAG);
    // Write to a file since cout and cerr maybe closed by the application
    OutFile << "Id,Instructions,Reads,Writes,Branches,FloatPoint" << endl;
    
    for (INT32 t=0; t<numThreads; t++)
    {
        thread_data_t* tdata = get_tls(t);
        OutFile << decstr(t) << "," << tdata->_count << "," << tdata->_reads << "," << tdata->_writes << "," << tdata->_branches << "," << tdata->_floatOps << endl;
    }

    OutFile.close();
    ReleaseLock(&OutFileLock);
}