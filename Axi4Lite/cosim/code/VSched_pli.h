//=====================================================================
//
// VSched_pli.h                                       Date: 2004/12/13
//
// Copyright (c) 2004-2010 Simon Southwell.
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
// $Id: VSched_pli.h,v 1.5 2021/05/15 07:45:17 simon Exp $
// $Source: /home/simon/CVS/src/HDL/VProc/code/VSched_pli.h,v $
//
//=====================================================================
//
// VSched function's export prototypes
//
//=====================================================================

#include <string.h>
#include "veriuser.h"

#ifdef INCL_VLOG_MEM_MODEL
#include "mem_model.h"
#endif

#ifndef _VSCHED_PLI_H_
#define _VSCHED_PLI_H_

#  ifdef DEBUG
#  define debug_io_printf printf
#  else
#  define debug_io_printf //
#  endif

#define VINIT_PARAMS       int  node
#define VSCHED_PARAMS      int  node, int Interrupt, int VPDataIn, int* VPDataOut, int* VPAddr, int* VPRw,int* VPTicks
#define VPROCUSER_PARAMS   int  node, int value
#define VACCESS_PARAMS     int  node, int  idx, int VPDataIn, int* VPDataOut
#define VHALT_PARAMS       int, int

#define VPROC_RTN_TYPE     void

extern VPROC_RTN_TYPE VInit     (VINIT_PARAMS);
extern VPROC_RTN_TYPE VSched    (VSCHED_PARAMS);
extern VPROC_RTN_TYPE VProcUser (VPROCUSER_PARAMS);
extern VPROC_RTN_TYPE VAccess   (VACCESS_PARAMS);
extern int            VHalt     (VHALT_PARAMS);

#endif
