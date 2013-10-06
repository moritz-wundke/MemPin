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

#ifndef MEMPIN_INSCOUNT_H
#define MEMPIN_INSCOUNT_H

//
// Tool entry points
//

BOOL inscount(INT32 toolId);
BOOL inscount_ext(INT32 toolId);

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56  // 64 byte line size: 64-8

// a running count of the instructions
class thread_data_t
{
  public:
    thread_data_t() : _count(0) {}
    UINT64 _count;
    UINT8 _pad[PADSIZE];
    UINT64 _reads;
    UINT64 _writes;
    UINT64 _branches;
    UINT64 _floatOps;
};

/** Get the datacainer of the given thread */
thread_data_t* get_tls(THREADID threadid);

/** Counts of instruction within a BBL */
VOID PIN_FAST_ANALYSIS_CALL inscount_docount(UINT32 c, THREADID threadid);

/** Catches when a thread gets started */
VOID Inscount_ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v);

/** Trace instrumentation callback */
VOID Inscount_Trace(TRACE trace, VOID *v);

/** Finish callback for basic inscount */
VOID Inscount_Fini(INT32 code, VOID *v);

/** Instruction instrumentation callback for extended inscount */
VOID Inscount_Ext_Instruction(INS ins, void *v);

/** Finish callback for extended inscount */
VOID Inscount_Ext_Fini(INT32 code, VOID *v);

#endif // MEMPIN_INSCOUNT_H