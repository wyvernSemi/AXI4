//=============================================================
//
// Copyright (c) 2022 Simon Southwell. All rights reserved.
//
// Date: 25th October 2022
//
// This file is part of the OSVVM Co-simulation features
//
// This is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The code is distributed in the hope that it will be useful,
// The code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "osvvm_cosim_skt_hdr.h"
#include "osvvm_cosim_skt.h"

// For Windows, need to link with Ws2_32.lib
#if defined (_WIN32) || defined (_WIN64)
// -------------------------------------------------------------------------
// INCLUDES (windows)
// -------------------------------------------------------------------------

# undef   UNICODE
# define  WIN32_LEAN_AND_MEAN

# include <windows.h>
# include <winsock2.h>
# include <ws2tcpip.h>

#else
// -------------------------------------------------------------------------
// INCLUDES (Linux)
// -------------------------------------------------------------------------

# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <termios.h>
#endif

#include "VUser.h"
#include "osvvm_cosim_skt_hdr.h"
#include "osvvm_cosim_skt.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// LOCAL CONSTANTS
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// STATIC VARIABLES
// -------------------------------------------------------------------------

osvvm_cosim_skt::osvvm_cosim_skt(const int port_number, const bool le) : portnum(port_number), ack_char(GDB_ACK_CHAR), rcvd_kill(false), little_endian(le)
{

    strncpy(hexchars, HEX_CHAR_MAP, str_buffer_size);

    if (init() < 0)
    {
        VPrint("osvvm_cosim_skt: ***ERROR initialising socket environment. Exiting...\n");
        exit(1);
    }

    // Create a TCP/IP socket
    if ((skt_hdl = connect_skt(portnum)) < 0)
    {
        VPrint("osvvm_cosim_skt: ***ERROR creating a TCP/IP socket. Exiting...\n");
        exit(2);
    }
}


// -------------------------------------------------------------------------
// osvvm_cosim_skt::init()
//
// Does any required socket initialisation, prior to opening a TCP socket.
// Current, only windows requires any handling.
//
// -------------------------------------------------------------------------

int osvvm_cosim_skt::init(void)
{
#if defined (_WIN32) || defined (_WIN64)
    WSADATA wsaData;

    // Initialize Winsock (windows only). Use windows socket spec. verions up to 2.2.
    if (int status = WSAStartup(MAKEWORD(VER_MAJOR, VER_MINOR), &wsaData))
    {
        VPrint("WSAStartup failed with error: %d\n", status);
        return OSVVM_COSIM_ERR;
    }
#endif

    return OSVVM_COSIM_OK;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::connect_skt()
//
// Opens a TCP socket connection, suitable for remote debugging, on the
// given port number (portno). It listens for a single connection, before
// returning the connection handle established. If any error occurs,
// OSVVM_COSIM_ERR is returned instead.
//
// -------------------------------------------------------------------------

osvvm_cosim_skt::osvvm_cosim_skt_t osvvm_cosim_skt::connect_skt (const int portno)
{

    // Create an IPv4 socket byte stream
    osvvm_cosim_skt_t svrskt;
    if ((svrskt = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
    {
        VPrint("ERROR opening socket\n");
        cleanup();
        return OSVVM_COSIM_ERR;
    }

    // Create and zero a server address structure
    struct sockaddr_in serv_addr;
    ZeroMemory((char *) &serv_addr, sizeof(serv_addr));

    // Configure the server address structure
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(portno);

    // Bind the socket to the address
    int status = bind(svrskt, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        VPrint("ERROR on Binding: %d\n", status);
        cleanup();
        return OSVVM_COSIM_ERR;
    }

    // Advertise the port number
    char errmsg[1024];
    
    sprintf(errmsg, "OSVVM_COSIM_SKT: Using TCP port number: %d\n", portno);
    io_printf(errmsg, portno);

    // Listen for connections (blocking)
    if (int status = listen(svrskt, MAXBACKLOG) < 0)
    {
        VPrint("ERROR on listening: %d\n", status);
        cleanup();
        return OSVVM_COSIM_ERR;
    }

    // Get a client address structure, and length as has to be passed as a pointer to accept()
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    // Accept a connection, and get returned handle
    osvvm_cosim_skt_t skt_hdl;
    if ((skt_hdl = accept(svrskt, (struct sockaddr *) &cli_addr,  &clilen)) < 0)
    {
        VPrint("ERROR on accept\n");
        cleanup();
        return OSVVM_COSIM_ERR;
    }

    // No longer need the server side (listening) socket
    closesocket(svrskt);

    // Return the handle to the connected socket. With this handle can
    // use recv()/send() to read and write (or, Linux only, read()/write()).
    return skt_hdl;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::cleanup()
//
// Does any open TCP socket cleanup before exiting the program. Current,
// only windows requires any handling.
//
// -------------------------------------------------------------------------

inline void osvvm_cosim_skt::cleanup(void)
{
#if defined (_WIN32) || defined (_WIN64)
    WSACleanup();
#endif
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::read_cmd()
//
// Read a byte from the socket and place in the buffer (buf). Return true
// on successful read, else return false. Compile dependent for windows
// (ReadFile) and linux (read)
//
// -------------------------------------------------------------------------

inline bool osvvm_cosim_skt::read_cmd (const osvvm_cosim_skt_t skt_hdl, char* buf)
{
    int status = OSVVM_COSIM_OK;

    // Read from the connection (up to 255 bytes plus null termination).
    if (recv(skt_hdl, buf, 1, 0) < 0)
    {
        VPrint("ERROR reading from socket\n");
        cleanup();
        status = OSVVM_COSIM_ERR;
    }

    return status == OSVVM_COSIM_OK;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::write_cmd()
//
// Write a byte to the socket from the buffer (buf). Return true
// on successful read, else return false. Compile dependent for windows
// (WriteFile) and linux (write)
//
// -------------------------------------------------------------------------

inline bool osvvm_cosim_skt::write_cmd (const osvvm_cosim_skt_t skt_hdl, const char* buf)
{
    int status = OSVVM_COSIM_OK;

    if (send(skt_hdl, buf, 1, 0) < 0)
    {
        VPrint("ERROR writing to socket\n");
        status = OSVVM_COSIM_ERR;
    }

    return status == OSVVM_COSIM_OK;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::read_mem()
//
// Read from memory in reply to a read type command (in cmd), and place
// reply in buf. Format of the command is
//
//   M addr,length
//
// Reply format is 'XX...', as set of hex character pairs for each byte, for
// as many as specified in length, starting from addr in memory. The
// checksum is calculated for the returned characters, and returned in
// checksum.
//
// -------------------------------------------------------------------------

int osvvm_cosim_skt::read_mem(const char* cmd, const int cmdlen, char *buf, unsigned char &checksum)
{
    int      bdx  = 0;
    int      cdx  = 0;
    uint64_t addr = 0;
    unsigned len  = 0;

    // Skip command character
    cdx++;

    // Get address
    while (cdx < cmdlen && cmd[cdx] != ',')
    {
        if (cmd[cdx] != ' ')
        {
            addr <<= 4;
            addr  |= CHAR2NIB(cmd[cdx]);
        }
        cdx++;
    }

    // Skip comma and any spaces
    while (cmd[cdx] == ',' || cmd[cdx] == ' ')
    {
        cdx++;
    }

    // Get length
    while (cdx < cmdlen)
    {
        len <<= 4;
        len  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    uint64_t val = 0x12345678;

    //////////////////////////////////////////////////////
    // Put co-sim calls here
    //////////////////////////////////////////////////////

#ifndef TEST
    switch(len*8)
    {
        uint32_t rdata32;
        uint16_t rdata16;
        uint8_t  rdata8;

        case 32: VTransRead((uint32_t)addr, (uint32_t*)&rdata32); val = rdata32; break;
        case 16: VTransRead((uint32_t)addr, (uint16_t*)&rdata16); val = rdata16; break;
        case  8: VTransRead((uint32_t)addr,  (uint8_t*)&rdata8);  val = rdata8;  break;
        default: return OSVVM_COSIM_ERR;
    }
#endif

    // Get memory bytes and put values as hex characters in buffer
    for (int idx = 0; idx < len; idx++)
    {
        bool fault;
        uint8_t byte;

        if (little_endian)
        {
            byte = (val >> (8*idx)) & 0xff; // Little endian
        }
        else
        {
            byte = (val >> (8*(len-idx-1))) & 0xff; // Big endian
        }

        checksum += buf[bdx++] = HIHEXCHAR(byte);
        checksum += buf[bdx++] = LOHEXCHAR(byte);
    }

    VPrint("read_mem 0x%08x (len = %d, addr=0x%08x)\n", (uint32_t)val, len, (uint32_t)addr);

    return bdx;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::write_mem()
//
// Write to memory in reply to a write type command (in cmd), and place
// a reply in buf. Format of the command is
//
//   M addr,length:XX...
//
// Where XX... is a set of hex character pairs for each byte to be written,
// for length bytes, starting at addr. The data may be in binary format
// (flagged by is_binary), in which case the XX data are single raw bytes,
// some of which are 'escaped' (see commants in function). As the memory
// write command's data can be large, the passed in cmd buffer does not
// contain the data after the ':' delimiter. This is read directly from
// the serial port and placed into the cpu's memory. A reply is placed in
// buf ("OK", or "EIO" if an error), with a calculated checksum returned
// in checksum.
//
// -------------------------------------------------------------------------

int osvvm_cosim_skt::write_mem (const osvvm_cosim_skt_t skt_hdl, const char* cmd, const int cmdlen, char *buf,
                                unsigned char &checksum, const bool is_binary)
{
    int      bdx          = 0;
    int      cdx          = 0;
    uint64_t addr         = 0;
    unsigned len          = 0;
    bool     io_status_ok = true;

    // Skip command character
    cdx++;

    // Get address
    while (cdx < cmdlen && cmd[cdx] != ',')
    {
        if (cmd[cdx] != ' ')
        {
            addr <<= 4;
            addr  |= CHAR2NIB(cmd[cdx]);
        }
        cdx++;
    }

    // Skip comma and any spaces
    while (cmd[cdx] == ',' || cmd[cdx] == ' ')
    {
        cdx++;
    }

    // Get length
    while (cdx < cmdlen && cmd[cdx] != ':')
    {
        len <<= 4;
        len  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Skip colon
    cdx++;

    uint64_t val = 0;

    // Get hex characters byte values and put into memory
    for (unsigned int idx = 0; idx < len; idx++)
    {

        if (little_endian)
        {
            val <<= 8;
        }
        else
        {
            val >>= 8;
        }

        char ipbyte[2];

        if (is_binary)
        {
            io_status_ok |= read_cmd(skt_hdl, ipbyte);

            val |= ipbyte[0];

            // Some binary data is escaped (with '}' character) and the following is the data
            // XORed with a pattern (0x20). '#', '$', and '}' are all escaped. Replies
            // containing '*' (0x2a) must be escaped. See 'Debugging with GDB' manual, Appendix E.1
            if (val == GDB_BIN_ESC)
            {
                io_status_ok |= read_cmd(skt_hdl, ipbyte);

                val |= ipbyte[0] ^ GDB_BIN_XOR_VAL;
            }
        }
        else
        {
            io_status_ok |= read_cmd(skt_hdl, &ipbyte[0]);
            io_status_ok |= read_cmd(skt_hdl, &ipbyte[1]);

            // Get byte value from hex
            uint8_t byte = 0;

            byte |= CHAR2NIB(ipbyte[0]) << 4;
            byte |= CHAR2NIB(ipbyte[1]);

            if (little_endian)
            {
                val |= byte;
            }
            else
            {
                val |= ((uint64_t)byte << ((len-idx-1)*8));
            }
        }

        if (!io_status_ok)
        {
            // On an error, break out of the loop
            break;
        }
    }

    //////////////////////////////////////////////////////
    // Put co-sim calls here
    //////////////////////////////////////////////////////

    VPrint("write_mem 0x%08x (len = %d, addr=0x%08x)\n", (uint32_t)val, len, (uint32_t)addr);

#ifndef TEST
    switch(len*8)
    {
        case 32: VTransWrite((uint32_t)addr, (uint32_t)val); break;
        case 16: VTransWrite((uint32_t)addr, (uint16_t)val); break;
        case  8: VTransWrite((uint32_t)addr,  (uint8_t)val); break;
        default: io_status_ok = false;
    }
#endif

    // Acknowledge the command
    if (io_status_ok)
    {
        BUFOK(buf, bdx, checksum);
    }
    else
    {
        BUFERR(EIO, buf, bdx, checksum);
    }

    return io_status_ok ? bdx : OSVVM_COSIM_ERR;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::proc_cmd()
//
// Processes a single command, as stored in cmd. The command is
// inspected and the appropriate local functions called. Generated replies
// are added to op_buf, with this function bracketing these with $ and #,
// followed by the two character checksum, returned by the functions (if
// any). An exception to a reply is for the kill (k) command which has
// no reply. Unsupported commands return a default reply of "$#00".
// The function sends the reply to the socket and then return either
// true if a 'detach' command (D) was seen, otherwise false.
//
// -------------------------------------------------------------------------

bool osvvm_cosim_skt::proc_cmd (const osvvm_cosim_skt_t skt_hdl, const char* cmd, const int cmdlen)
{
    int           op_idx    = 0;
    unsigned char checksum  = 0;
    bool          detached  = false;
    static int    reason    = 0;

    // Packet start
    op_buf[op_idx++] = GDB_SOP_CHAR;

//#ifdef OSVVM_COSIM_DEBUG
    VPrint("CMD = %s\n", cmd);
//#endif

    // Select on command character
    switch(cmd[0])
    {
        int status;

    // Read memory
    case 'm':
        status = read_mem(cmd, cmdlen, &op_buf[op_idx], checksum);

        if (status != OSVVM_COSIM_ERR)
        {
            op_idx += status;
        }
        else
        {
            detached = true;
            BUFERR(EIO, op_buf, op_idx, checksum);
            VPrint("***ERROR: proc_cmd() read memory failed\n");
        }
        break;

    // Write memory (binary)
    case 'X':
        status = write_mem(skt_hdl, cmd, cmdlen, &op_buf[op_idx], checksum, true);

        if (status != OSVVM_COSIM_ERR)
        {
            op_idx += status;
        }
        else
        {
            detached = true;
            VPrint("***ERROR: proc_cmd() write memory failed\n");
        }
        break;

    // Write memory
    case 'M':
        status = write_mem(skt_hdl, cmd, cmdlen, &op_buf[op_idx], checksum, false);

        if (status != OSVVM_COSIM_ERR)
        {
            op_idx += status;
        }
        else
        {
            detached = true;
            VPrint("***ERROR: proc_cmd() write memory failed\n");
        }
        break;

    case 'D':
        detached = true;
        BUFOK(op_buf, op_idx, checksum);
        break;

    case 'k':
        rcvd_kill = true;
        detached  = true;
        break;
    }

    // Packet end
    op_buf[op_idx++] = GDB_EOP_CHAR;

    // Checksum
    op_buf[op_idx++] = HIHEXCHAR(checksum);
    op_buf[op_idx++] = LOHEXCHAR(checksum);

    // Terminate buffer with a NULL character in case we want to print for debug
    op_buf[op_idx]   = 0;

    // Send reply if not 'kill' command (which has no reply)
    if (!rcvd_kill)
    {
        // Output the response for the command to the terminal
        for (int idx = 0; idx < op_idx; idx++)
        {
            if (!write_cmd(skt_hdl, &op_buf[idx]))
            {
                VPrint("OSVVM_COSIM_SKT: ERROR writing to host: terminating.\n");
                return true;
            }
        }

//#ifdef OSVVM_COSIM_DEBUG
        VPrint("\nREPLY: %s\n", op_buf);
//#endif

    }

    return detached;
}

// -------------------------------------------------------------------------
// osvvm_cosim_skt::process_pkt()
//
// Top level for the socket interface of OSVVM cosim features.
//
// It calls local functions to create a pseudo/virtual serial port for GDB
// connection, and starts reading characters for this port. It monitors
// for start and end of packets, placing packet contents in ip_buf. Once
// a whole packet is received, it calls osvvm_cosim_proc_cmd() to process
// it. This repeats until osvvm_cosim_proc_cmd() returns true, flagging
// that the GDB session has detached, when the function cleans up and
// returns. It will return OSVVM_COSIM_OK if all is well, else OSVVM_COSIM_ERR
// is returned.
//
// -------------------------------------------------------------------------

int osvvm_cosim_skt::process_pkts (void)
{
    int   idx      = 0;
    bool  active   = false;
    bool  detached = false;
    bool  waiting  = true;
    char  ipbyte;

    while (!detached && read_cmd(skt_hdl, &ipbyte))
    {
        // If waiting for first communication, flag that attachment has happened.
        if (waiting)
        {
            waiting = false;
            VPrint("OSVVM_COSIM_SKT: host attached.\n");
            fflush(stderr);
        }

        // If receiving a packet end character (or delimiter for mem writes), process the command an go idle
        if (active && (ipbyte  == GDB_EOP_CHAR     ||
                       idx     == ip_buffer_size-1 ||
                       (ipbyte == GDB_MEM_DELIM_CHAR && (ip_buf[0] == 'X' || ip_buf[0] == 'M'))))
        {
            // Acknowledge the packet
            if (!write_cmd(skt_hdl, &ack_char))
            {
                return OSVVM_COSIM_ERR;
            }

            // Terminate the buffer string, for ease of debug
            ip_buf[idx] = 0;

            // Process the command
            detached = proc_cmd(skt_hdl, ip_buf, idx);

            // Flag state as inactive
            active = false;

            // Reset input buffer index
            idx    = 0;

#ifdef OSVVM_COSIM_DEBUG
            // At termination echo newline char to stdout
            putchar('\n');
            fflush(stdout);
#endif
        }
        // Wait for a packet start character
        else if (!active && ipbyte == GDB_SOP_CHAR)
        {
            active = true;
        }
        // Get command packet characters, store in buffer [and echo to screen].
        else if (active)
        {
            ip_buf[idx++] = ipbyte;

#ifdef OSVVM_COSIM_DEBUG
            // Echo packet data to stdout
            putchar(ipbyte);
            fflush(stdout);
#endif
        }
    }

    if (detached)
    {
        VPrint("OSVVM_COSIM_SKT: host %s from target: terminating.\n", rcvd_kill ? "received 'kill'" : "detached");
    }
    else
    {
        VPrint("OSVVM_COSIM_SKT: connection lost to host: terminating.\n");
    }

    // Close socket of TCP connection
    closesocket(skt_hdl);

    return OSVVM_COSIM_OK;
}

