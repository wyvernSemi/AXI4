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
#include <cmath>
#include <algorithm>
#include <vector>

// Import VProc user API
#include "OsvvmVUser.h"

// I am node 0 context
static int node  = 0;

#ifdef _WIN32
#define srandom srand
#define random rand
#endif

typedef struct {
    uint32_t addr;
    uint32_t wdata;
    uint32_t size;
} wtrans_t;

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

static void logGdbMsg(FILE *fp, wtrans_t &w, bool rnw)
{
    char msg[256];

    if (rnw)
    {
        sprintf(msg, "m%x,%d\n", w.addr, w.size/8);
    }
    else
    {
        int byteSize = w.size/8;
        switch(w.size)
        {
        case 32: sprintf(msg, "M%x,%d:%08x\n", w.addr, byteSize, w.wdata); break;
        case 16: sprintf(msg, "M%x,%d:%04x\n", w.addr, byteSize, w.wdata & 0xffff); break;
        case  8: sprintf(msg, "M%x,%d:%02x\n", w.addr, byteSize, w.wdata & 0xff); break;
        }
    }

    fprintf(fp, "%s", msg);
}

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

    std::vector<wtrans_t> vec;
    wtrans_t              wtrans;

    // Use node number, inverted, as the random number generator seed.
    srandom(~node);

    FILE* fp = fopen("sktscript.txt", "w");

    while (true)
    {
        // Get read/write and address from random value
        uint32_t rnw = (uint32_t)(random() & 0x1UL);        // Bit 0

        // If no writes outstanding, force to be a write
        if (vec.empty())
        {
            rnw = 0;
        }

        // Do a read
        if (rnw)
        {
            uint32_t rdata;
            uint16_t rdata16;
            uint8_t  rdata8;

            char *msg = "?????" ;

            wtrans = vec.front();

            vec.erase(vec.begin());

            // Do a 32-bit read and place returned data in rdata
            switch(wtrans.size)
            {
                case  8: VTransRead(wtrans.addr, &rdata8);  rdata = rdata8;  msg = "byte";  break;
                case 16: VTransRead(wtrans.addr, &rdata16); rdata = rdata16; msg = "hword"; break;
                case 32: VTransRead(wtrans.addr, &rdata);                    msg = "word";  break;
            }

            if (rdata == wtrans.wdata)
            {
                // Display read transaction information
                VPrint("VUserMain0: read %s %08X from address %08X\n", msg, rdata, wtrans.addr);
            }
            else
            {
                VPrint("VUserMain0: ***ERROR*** read %s %08X from address %08X. Expected %08x\n", msg, rdata, wtrans.addr, wtrans.wdata);
            }
        }
        // Do a write
        else
        {
            char *msg = "?????" ;

            uint32_t log_size  = pow(2, (random() & 0x3)%3);
            uint32_t addr_mask = ~(log_size - 1UL);

            // Generate some random address
            wtrans.addr = (random() ^ (random() << 16)) & addr_mask;

            wtrans.size  = 8 * log_size;

            // Generate some random data
            wtrans.wdata = (random() ^ (random() << 16)) & ((1ULL << wtrans.size) - 1ULL);

            vec.push_back(wtrans);

            // Do a 32-bit write transaction
            switch(wtrans.size)
            {
                case  8: VTransWrite(wtrans.addr,  (uint8_t)wtrans.wdata); msg = "byte";  break;
                case 16: VTransWrite(wtrans.addr, (uint16_t)wtrans.wdata); msg = "hword"; break;
                case 32: VTransWrite(wtrans.addr, (uint32_t)wtrans.wdata); msg = "word";  break;
            }

            // Display write transaction display information
            VPrint("VUserMain0: wrote %s %08X to address %08X\n", msg, wtrans.wdata, wtrans.addr);
        }

        logGdbMsg(fp, wtrans, rnw);
    }

    fclose(fp);

    // If ever got this far then sleep forever
    SLEEPFOREVER;
}

