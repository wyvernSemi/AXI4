//=====================================================================
//
// VUser.h                                            Date: 2005/01/10
//
// Copyright (c) 2005-2010 Simon Southwell.
//
// This file is part of VProc.
//
// VProc is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VProc is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VProc. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: VUser.h,v 1.5 2021/05/04 15:38:37 simon Exp $
// $Source: /home/simon/CVS/src/HDL/VProc/code/VUser.h,v $
//
//=====================================================================

#ifndef _VUSER_H_
#define _VUSER_H_

#include <stdint.h>

#ifdef __cplusplus
#define EXTC  "C"
extern "C"
{
#else
#define EXTC
#endif

#include "VProc.h"
#include "VSched_pli.h"

#ifdef __cplusplus
}
#endif

#define DELTA_CYCLE     -1
#define NO_DELTA_CYCLE  0
#define GO_TO_SLEEP     0x7fffffff
#define SLEEPFOREVER    { while(1) VTick(GO_TO_SLEEP, node); }

#define MAX_INT_LEVEL   7
#define MIN_INT_LEVEL   1

// Pointer to pthread_create compatible function
typedef void *(*pThreadFunc_t)(void *);

// VUser function prototypes
#ifdef __cplusplus
// VProc write and read functions (fixed at 32-bit)
extern int      VWrite         (uint32_t addr, uint32_t  data, int delta, uint32_t node);
extern int      VRead          (uint32_t addr, uint32_t *data, int delta, uint32_t node);

// Overloaded write and read transaction functions for 32 bit architecture for byte,
// half-word and, words
extern uint8_t  VTransWrite    (uint32_t addr, uint8_t   data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint32_t addr, uint8_t  *data, int prot = 0, uint32_t node = 0);
extern uint16_t VTransWrite    (uint32_t addr, uint16_t  data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint32_t addr, uint16_t *data, int prot = 0, uint32_t node = 0);
extern uint32_t VTransWrite    (uint32_t addr, uint32_t  data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint32_t addr, uint32_t *data, int prot = 0, uint32_t node = 0);

// Overloaded write and read transaction functions for 64 bit architecture for byte,
// half-word, word, and double-word
extern uint8_t  VTransWrite    (uint64_t addr, uint8_t   data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint64_t addr, uint8_t  *data, int prot = 0, uint32_t node = 0);
extern uint16_t VTransWrite    (uint64_t addr, uint16_t  data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint64_t addr, uint16_t *data, int prot = 0, uint32_t node = 0);
extern uint32_t VTransWrite    (uint64_t addr, uint32_t  data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint64_t addr, uint32_t *data, int prot = 0, uint32_t node = 0);
extern uint64_t VTransWrite    (uint64_t addr, uint64_t  data, int prot = 0, uint32_t node = 0);
extern void     VTransRead     (uint64_t addr, uint64_t *data, int prot = 0, uint32_t node = 0);
#endif

extern EXTC int  VUser         (int node);
extern EXTC int  VTick         (uint32_t ticks, uint32_t node);
extern EXTC void VRegInterrupt (int level, pVUserInt_t func, uint32_t node);
extern EXTC void VRegUser      (pVUserCB_t func, uint32_t node);
extern EXTC void VRegUser      (pVUserCB_t func, uint32_t node);

// In windows using the FLI, a \n in the printf format string causes
// two lines to be advanced, so replace new lines with carriage returns
// which seems to work
# ifdef _WIN32
# define VPrint(format, ...) {int len;                                             \
                              char formbuf[256];                                   \
                              strncpy(formbuf, format, 255);                       \
                              len = strlen(formbuf);                               \
                              for(int i = 0; i < len; i++)                         \
                                if (formbuf[i] == '\n')                            \
                                  formbuf[i] = '\r';                               \
                              printf (formbuf, ##__VA_ARGS__);                     \
                              }
# else
# define VPrint(...) io_printf (__VA_ARGS__)
# endif

#ifdef DEBUG
#define DebugVPrint VPrint
#else
#define DebugVPrint(...) {}

#endif

// Pointer to VUserMain function type definition
typedef void (*pVUserMain_t)(void);

#endif
