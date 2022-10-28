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

#ifndef _OSVVM_COSIM_SKT_HDR_H_
#define _OSVVM_COSIM_SKT_HDR_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#if defined (_WIN32) || defined (_WIN64)

// -------------------------------------------------------------------------
// DEFINITIONS (windows)
// -------------------------------------------------------------------------

// Define the max supported winsocket spec. major and minor numbers
# define VER_MAJOR           2
# define VER_MINOR           2

#else

// -------------------------------------------------------------------------
// DEFINITIONS (Linux)
// -------------------------------------------------------------------------

// Map some windows function names to the file access Linux equivalents
# define closesocket         close
# define ZeroMemory          bzero

#endif

#define GDB_ACK_CHAR         '+'
#define GDB_NAK_CHAR         '-'
#define GDB_SOP_CHAR         '$'
#define GDB_EOP_CHAR         '#'
#define GDB_MEM_DELIM_CHAR   ':'
#define GDB_BIN_ESC          0x7d
#define GDB_BIN_XOR_VAL      0x20
#define HEX_CHAR_MAP         "0123456789abcdef"
#define MAXBACKLOG           5

// -------------------------------------------------------------------------
// MACRO DEFINITIONS
// -------------------------------------------------------------------------

#define HIHEXCHAR(_x) hexchars[((_x) & 0xf0) >> 4]
#define LOHEXCHAR(_x) hexchars[ (_x) & 0x0f]

#define CHAR2NIB(_x) (((_x) >= '0' && (_x) <= '9') ? (_x) - '0': \
                      ((_x) >= 'a' && (_x) <= 'f') ? (10 + (_x) - 'a'): \
                                                     (10 + (_x) - 'A'))

#define BUFBYTE(_b,_i,_v) {     \
    _b[_i]     = HIHEXCHAR(_v); \
    _b[(_i)+1] = LOHEXCHAR(_v); \
    _i        += 2;             \
}
#define BUFWORD(_b,_i,_v) {     \
    BUFBYTE(_b,_i,(_v) >> 24);  \
    BUFBYTE(_b,_i,(_v) >> 16);  \
    BUFBYTE(_b,_i,(_v) >>  8);  \
    BUFBYTE(_b,_i,(_v) >>  0);  \
}

#define BUFWORDLE(_b,_i,_v) {   \
    BUFBYTE(_b,_i,(_v) >>  0);  \
    BUFBYTE(_b,_i,(_v) >>  8);  \
    BUFBYTE(_b,_i,(_v) >> 16);  \
    BUFBYTE(_b,_i,(_v) >> 24);  \
}

#define BUFOK(_b,_i,_c) {_c += _b[_i] = 'O'; _c += _b[_i+1] = 'K'; _i+=2;}

#define BUFERR(_e,_b,_i,_c) {_c += _b[_i] = 'E'; _c += _b[_i+1] = HIHEXCHAR(_e); _c += _b[_i+2] = LOHEXCHAR(_e); _i+=3;}

// -------------------------------------------------------------------------
// TYPE DEFINITIONS
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// ENUMERATIONS
// -------------------------------------------------------------------------

#endif
