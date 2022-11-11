// -------------------------------------------------------------------------
// VUserMain0()
//
// Entry point for OSVVM co-simulation code for node 0
//
// This function creates a socket object which opens a TCP/IP server socket
// and starts listening. When the process_pkts() method is called it will 
// process gdb remote serial interface commands for memory reads and writes
// up to 64 bits, calling the co-sim API to instigate bus transactions on
// OSVVM.
//
// -------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "OsvvmVUser.h"
#include "OsvvmCosimSkt.h"

static int node = 0;

#ifdef TEST

extern "C" int VTick(uint32_t, uint32_t)
{
    exit(0);
}

#endif

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

extern "C" void VUserMain0()
{
    OsvvmCosimSkt skt;

    if (skt.ProcessPkts() != OsvvmCosimSkt::OSVVM_COSIM_OK)
    {
        fprintf(stderr, "***ERROR: socket exited with bad status\n");
    }
    else
    {
        printf("DONE\n");
    }

    SLEEPFOREVER;

}

#ifdef TEST
int main (int argc, char* argv[])
{
    VUserMain0();

    return 0;
}

#endif