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
#include "rv32.h"
#include "rv32_cpu_gdb.h"

static int node = 0;

// Type definition for write transaction, for use in TCP/IP socket script generation
typedef struct {
    uint32_t addr;
    uint32_t wdata;
    uint32_t size;
} wtrans_t;

// Local file pointer for optionally generating TCP/IP socket script file for each transaction
static FILE *sktfp = NULL;

// -------------------------------------------------------------------------
// Dump registers using calls to rv32 object
// -------------------------------------------------------------------------

static void reg_dump(rv32* pCpu, FILE* dfp, bool abi_en)
{
    fprintf(dfp, "\nRegister state:\n\n  ");

    // Loop through all the registers
    for (int idx = 0; idx < RV32I_NUM_OF_REGISTERS; idx++)
    {
        // Get the appropriate mapped register name (ABI or x)
        const char* map_str = abi_en ? pCpu->rmap_str[idx] : pCpu->xmap_str[idx];

        // Get the length of the register name string
        size_t  slen = strlen(map_str);

        // Fetch the value of the register indexed
        uint32_t rval = pCpu->regi_val(idx);

        // Print out the register name (right justified) followed by the value
        fprintf(dfp, "%s%s = 0x%08x ", (slen == 2) ? "  " : (slen == 3) ? " ": "",
                                         map_str,
                                         rval);

        // After every fourth value, output a new line
        if ((idx % 4) == 3)
        {
            fprintf(dfp, "\n  ");
        }
    }

    // Add a final new line
    fprintf(dfp, "\n");
}

// -------------------------------------------------------------------------
// Dump memory
// -------------------------------------------------------------------------

static void mem_dump(uint32_t num, uint32_t start, rv32* pCpu, FILE* dfp)
{
    bool fault;

    fprintf(dfp, "\nMEM state:\n\n");
    for (uint32_t idx = start; idx < ((start & 0xfffffffc) + num*4); idx+=4)
    {
        uint32_t rval = pCpu->read_mem(idx, MEM_RD_ACCESS_WORD, fault);
        fprintf(dfp, "  0x%08x : 0x%08x\n", idx, rval);
    }
    fprintf(dfp, "\n");
}

// ------------------------------------------------------------------------------
// Generate a socket script based on transacion struct and rnw flag.
// ------------------------------------------------------------------------------

static void logGdbMsg(FILE *fp, wtrans_t &w, bool rnw)
{
    char msg[256];

    if (rnw)
    {
        sprintf(msg, "m%08x,%d\n", w.addr, w.size/8);
    }
    else
    {
        int byteSize = w.size/8;
        switch(w.size)
        {
        case 32: sprintf(msg, "M%08x,%d:%08x\n", w.addr, byteSize, w.wdata); break;
        case 16: sprintf(msg, "M%08x,%d:%04x\n", w.addr, byteSize, w.wdata & 0xffff); break;
        case  8: sprintf(msg, "M%08x,%d:%02x\n", w.addr, byteSize, w.wdata & 0xff); break;
        }
    }

    fprintf(fp, "%s", msg);
}

// -------------------------------------------------------------------------
// ISS memory access callback function
// -------------------------------------------------------------------------

static int memcosim (const uint32_t byte_addr, uint32_t &data, const int type, const rv32i_time_t time)
{
    int      cycle_count = 5;
    uint8_t  rdata8;
    uint16_t rdata16;
    uint32_t rdata32;
    wtrans_t trans;

    // Select the co-simulation call based on the access type
    switch(type)
    {
        case MEM_WR_ACCESS_BYTE  : VTransWrite(byte_addr,  (uint8_t)data)           ; break;
        case MEM_WR_ACCESS_HWORD : VTransWrite(byte_addr, (uint16_t)data)           ; break;
        case MEM_WR_ACCESS_WORD  : VTransWrite(byte_addr, (uint32_t)data)           ; break;
        case MEM_WR_ACCESS_INSTR : VTransWrite(byte_addr, (uint32_t)data)           ; break;
        case MEM_RD_ACCESS_BYTE  : VTransRead(byte_addr,  &rdata8);  data = rdata8  ; break;
        case MEM_RD_ACCESS_HWORD : VTransRead(byte_addr,  &rdata16); data = rdata16 ; break;
        case MEM_RD_ACCESS_WORD  : VTransRead(byte_addr,  &rdata32); data = rdata32 ; break;
        case MEM_RD_ACCESS_INSTR : VTransRead(byte_addr,  &rdata32); data = rdata32 ; break;
        default: cycle_count = RV32I_EXT_MEM_NOT_PROCESSED; break;
    }

    // If the socket script file point is active, save off a TCP/IP transaction packet command
    if (sktfp != NULL)
    {
        bool rnw = (type >= MEM_RD_ACCESS_BYTE);
        
        trans.addr = byte_addr;
        trans.wdata = data;
        trans.size  = (type == MEM_WR_ACCESS_BYTE  || type == MEM_RD_ACCESS_BYTE)  ?  8 :
                      (type == MEM_WR_ACCESS_HWORD || type == MEM_RD_ACCESS_HWORD) ? 16 :
                                                                                     32 ;

        logGdbMsg(sktfp, trans, rnw);
    }

    return cycle_count;
}

// =========================================================================
// Main entry point for co-simulation node 0
// =========================================================================

extern "C" void VUserMain0()
{
    // Create a configuration object
    rv32i_cfg_s cfg;

    // Override the defaults with test specifics

    // Configure halt conditions
    cfg.hlt_on_ecall         = true;

    // Configure program to run
    cfg.user_fname           = true;
    cfg.exec_fname           = "test.exe";

    // Configure debug output
    cfg.rt_dis               = true;
    cfg.abi_en               = true;
    cfg.dump_regs            = true;

    // Configure GDB debugging mode
    cfg.gdb_mode             = false;
    cfg.gdb_ip_portnum       = 0xc000;
    
    // Open up a socket script file
    sktfp = fopen("sktscript.txt", "w");

    // Create a new cpu object
    rv32* pCpu               = new rv32();

    pCpu->register_ext_mem_callback(memcosim);

    // If GDB mode not configured, simply run the specified program
    if (!cfg.gdb_mode)
    {
        // Load an executable
        if (!pCpu->read_elf(cfg.exec_fname))
        {
            // Run processor until halted
            pCpu->run(cfg);

            // Check exit status
            if (pCpu->regi_val(10) || pCpu->regi_val(17) != 93)
            {
                VPrint("*FAIL*: exit code = 0x%08x finish code = 0x%08x running %s\n",
                        pCpu->regi_val(10) >> 1, pCpu->regi_val(17), cfg.exec_fname);
            }
            else
            {
                VPrint("PASS: exit code = 0x%08x running %s\n", 
                        pCpu->regi_val(10), cfg.exec_fname);
            }

            // If enabled, dump the registers
            if (cfg.dump_regs)
            {
                reg_dump(pCpu, stdout, cfg.abi_en);
            }

            // If specified dump the numbr of DMEM words
            if (cfg.num_mem_dump_words)
            {
                mem_dump(cfg.num_mem_dump_words,
                         cfg.mem_dump_start,
                         pCpu,
                         stdout);
            }

        }

    }
    // If GDB mode configured, run with that
    else
    {
#ifdef __WIN32__
        WORD versionWanted   = MAKEWORD(1, 1);
        WSADATA wsaData;
        WSAStartup(versionWanted, &wsaData);
#endif
        int error = 0;

        // Load an executable if specified in the configuration
        if (cfg.user_fname)
        {
            if (pCpu->read_elf(cfg.exec_fname))
            {
                error        = 1;
            }
        }

        if (!error)
        {
            // Start procssing commands from GDB
            if (rv32gdb_process_gdb(pCpu, cfg.gdb_ip_portnum, cfg))
            {
                fprintf(stderr, "***ERROR in opening PTY\n");
            }
        }

#ifdef __WIN32__
        WSACleanup;
#endif
    }

    // Clean up
    if (cfg.dbg_fp != stdout)
    {
        fclose(cfg.dbg_fp);
    }
    delete pCpu;

    SLEEPFOREVER;
}

