//=====================================================================
//
// VProc.h                                            Date: 2004/12/13 
//
// Copyright (c) 2004-2022 Simon Southwell.
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
//=====================================================================
//
// Internal header for VProc definitions and data types
//
//=====================================================================

#ifndef _VPROC_H_
#define _VPROC_H_

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#ifndef __USE_GNU
#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU
#else
#include <dlfcn.h>
#endif

#include <pthread.h>
#include <sched.h>

#include <semaphore.h>

// For file IO
#include <fcntl.h>

// For inode manipulation
#include <unistd.h>

#ifndef VP_MAX_NODES
#define VP_MAX_NODES            64
#endif


#define V_IDLE                  0
#define V_WRITE                 1
#define V_READ                  2
#define V_HALT                  4
#define V_SWAP                  8

#define VP_EXIT_OK              0
#define VP_QUEUE_ERR            1
#define VP_KEY_ERR              2
#define VP_USER_ERR             3
#define VP_SYSCALL_ERR          4

#define UNDEF                   -1

#define DEFAULT_STR_BUF_SIZE    32

#define MIN_INTERRUPT_LEVEL     1
#define MAX_INTERRUPT_LEVEL     7


typedef enum trans_type_e
{
  trans_wr_byte           = 0,
  trans_wr_hword          = 1,
  trans_wr_word           = 2,
  trans_wr_dword          = 3,
  trans_wr_qword          = 4,
  trans_rd_byte           = 8,
  trans_rd_hword          = 9,
  trans_rd_word           = 10,
  trans_rd_dword          = 11,
  trans_rd_qword          = 12
} trans_type_e;

typedef enum arch_e
{
    arch32,
    arch64,
    arch128
} arch_e;

typedef struct
{
    trans_type_e        type;
    uint32_t            prot;
    uint32_t            addr;
    uint8_t             data[16];
    uint32_t            rw;
    int                 ticks;
} send_buf_t, *psend_buf_t;

typedef struct
{
    unsigned int        data_in;
    void*               data;
    unsigned int        interrupt;
} rcv_buf_t, *prcv_buf_t;


// Shared object handle type
typedef void * handle_t;

// Interrupt function pointer type
typedef int  (*pVUserInt_t)      (void);
typedef int  (*pVUserCB_t)       (int);

typedef struct
{
    sem_t               snd;
    sem_t               rcv;
    send_buf_t          send_buf;
    rcv_buf_t           rcv_buf;
    pVUserInt_t         VInt_table[MAX_INTERRUPT_LEVEL+1];
    pVUserCB_t          VUserCB;
} SchedState_t, *pSchedState_t;

extern pSchedState_t ns[VP_MAX_NODES];

#endif
