//=============================================================
//
// Copyright (c) 2022 Simon Southwell. All rights reserved.
//
// Date: 25th October 2022
//
// This file is part of the OSVVM co-simulation features
//
// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
//=============================================================

#ifndef _OSVVM_COSIM_SKT_H_
#define _OSVVM_COSIM_SKT_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#if defined (_WIN32) || defined (_WIN64)

#include <Winsock2.h>

#endif

// -------------------------------------------------------------------------
// CLASS DEFINITION
// -------------------------------------------------------------------------

class osvvm_cosim_skt
{
public:
           static const int  OSVVM_COSIM_OK      = 0;
           static const int  OSVVM_COSIM_ERR     = -1;

private:
           static const int  default_tcp_portnum = 0xc000;
           static const int  ip_buffer_size      = 1024;
           static const int  op_buffer_size      = 1024;
           static const int  str_buffer_size     = 100;

public:
    // Constructor
                             osvvm_cosim_skt (const int  port_number = default_tcp_portnum,
                                              const bool le          = true) ;

    // User entry point method
           int               process_pkts    (void);

private:

#if defined (_WIN32) || defined (_WIN64)
           // Map the socket type for windows
           typedef SOCKET    osvvm_cosim_skt_t;
#else
           // Map the socket type for Linux
           typedef long long osvvm_cosim_skt_t;
#endif

    // Private methods
    
           // Methods for managing the socket connection
           int               init            (void);
           osvvm_cosim_skt_t connect_skt     (const int portno);
           void              cleanup         (void);

           // Methods for processing commands
           bool              proc_cmd        (const osvvm_cosim_skt_t skt_hdl, const char* cmd, const int cmdlen);
           bool              read_cmd        (const osvvm_cosim_skt_t skt_hdl, char* buf);
           bool              write_cmd       (const osvvm_cosim_skt_t skt_hdl, const char* buf);

           // Methods to do the memory mapped accesses
           int               read_mem        (const char* cmd, const int cmdlen, char *buf, unsigned char &checksum);
           int               write_mem       (const osvvm_cosim_skt_t skt_hdl,   const char* cmd, const int cmdlen, char *buf,
                                              unsigned char &checksum, const bool is_binary);

    // Private member variables
           bool              rcvd_kill;
           char              ip_buf[ip_buffer_size];
           char              op_buf[op_buffer_size];
           osvvm_cosim_skt_t skt_hdl;
    const  int               portnum;
    const  char              ack_char;
           char              hexchars[str_buffer_size];

           bool              little_endian;

};

#endif
