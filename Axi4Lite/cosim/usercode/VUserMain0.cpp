// ------------------------------------------------------------------------------
//
//  File Name:           VUserMain0.cpp
//  Design Unit Name:    Co-simulation virtual processor test program
//  Revision:            OSVVM MODELS STANDARD VERSION
//
//  Maintainer:          Simon Southwell      email:  simon.southwell@gmail.com
//  Contributor(s):
//     Simon Southwell   simon.southwell@gmail.com
//
//  Description:
//      Co-simulation test transaction source
//
//  Developed by:
//        Simon Southwell
//
//  Revision History:
//    Date      Version    Description
//    09/2022   2022       Initial revision
//
//  This file is part of OSVVM.
//
//  Copyright (c) 2022 by Simon Southwell
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// ------------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstdint>

// Import VProc user API
#include "VUser.h"

// I am node 0 context
static int node  = 0;

#ifdef _WIN32
#define srandom srand
#define random rand
#endif

// ------------------------------------------------------------------------------
// Main entry point for node 0 virtual processor software
//
// VUserMainX has no calling arguments. If runtime configuration required
// then you'll need to read in a configuration file.
//
// ------------------------------------------------------------------------------

extern "C" void VUserMain0()
{
    VPrint("VUserMain0(): node=%d\n", node);

    // Use node number plus 1 as the ransom number generator seed.
    srandom(node + 1);

    while (true)
    {
        // Generate a random number
        long int randval = random();
        
        // Get read/write and address from random value
        uint32_t rnw  = (uint32_t)(randval & 0x1UL);        // Bit 0
        uint32_t addr = (uint32_t)(randval & 0xfffffffcUL); // Bits 31:2

        // Do a read
        if (rnw)
        {
            uint32_t rdata;
            
            // Do a 32-bit read and place returned data in rdata
            VTransRead(addr, &rdata);
            
            // Display read transaction information
            VPrint("VUserMain0: read %08X from address %08X\n", rdata, addr);
        }
        // Do a write
        else
        {
            // Generate some random data
            uint32_t wdata = random() & 0xffffffffUL;
            
            // Do a 32-bit write transaction
            VTransWrite(addr, wdata);
            
            // Display write trabsaction display information
            VPrint("VUserMain0: wrote %08X to address %08X\n", wdata, addr);
        }
    }

    // If ever got this far then sleep forever
    SLEEPFOREVER;
}

