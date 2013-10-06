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

#ifndef MEMPIN_H
#define MEMPIN_H

/** STD includes */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

/** Pin includes */
#include "pin.H"

/** Our includes */
#include "mempin_utils.h"
#include "mempin_tools.h"

// The process pid
extern INT32 gPinPid;

// The output file
extern ofstream OutFile;
extern PIN_LOCK OutFileLock;

// The lock tag when using the output file and we are not in a thread
#define BASE_LOCK_TAG 0

/** Strip path from string */
const char * StripPath(const char * path);
//
// Tool register. Just a list of function pointers which will the
// initialize the actual tool
// 
typedef BOOL (*tool_init)(INT32);

extern std::vector<tool_init> gToolRegister;

/** Register a tool */
void register_tool(tool_init tool);

/** Start the given tool with the given ID */
void start_tool(INT32 toolId);

/** Add all our tools */
void register_tools();

#endif // MEMPIN_H