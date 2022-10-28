###################################################################
# Makefile for Virtual Processor testcode in Modelsim
#
# Copyright (c) 2005-2010 Simon Southwell.
#
# This file is part of VProc.
#
# VProc is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# VProc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with VProc. If not, see <http://www.gnu.org/licenses/>.
#
###################################################################

# $MODEL_TECH environment variable must be set

# Define the maximum number of supported VProcs in the compile pli library
MAX_NUM_VPROC      = 64

SRCDIR             = code
USRCDIR            = usercode
TESTDIR            = .
VOBJDIR            = ${TESTDIR}/obj
OPDIR              = ${TESTDIR}/../sim
MEMMODELDIR        = .

# VPROC C source code
VPROC_C            = $(wildcard ${SRCDIR}/*.c*)
VPROC_C_BASE       = $(notdir $(filter %c, ${VPROC_C}))
VPROC_CPP_BASE     = $(notdir $(filter %cpp, ${VPROC_C}))

# Memory model C code
MEM_C              =

# Test user code
USER_C             = $(wildcard ${USRCDIR}/*.c*)

USER_CPP_BASE      = $(notdir $(filter %cpp, ${USER_C}))
USER_C_BASE        = $(notdir $(filter %c, ${USER_C}))

SRC_INCL           = $(wildcard ${SRCDIR}/*.h)
USER_INCL          = $(wildcard ${USRCDIR}/*.h)

VOBJS              = $(addprefix ${VOBJDIR}/, ${USER_C_BASE:%.c=%.o} ${USER_CPP_BASE:%.cpp=%.o} \
                     ${VPROC_C_BASE:%.c=%.o} ${VPROC_CPP_BASE:%.cpp=%.o})

USRFLAGS           =

# Generated  PLI C library
VPROC_PLI          = ${OPDIR}/VProc.so
VLIB               = ${TESTDIR}/libvproc.a

# Get OS type
OSTYPE:=$(shell uname)

# Set OS specific variables between Linux and Windows (MinGW)
ifeq (${OSTYPE}, Linux)
  CFLAGS_SO        = -shared -lpthread -lrt -rdynamic
  CPPSTD           = -std=c++11
  MODELSIMBINDIR   = linuxaloem
  WLIB             =
else
  CFLAGS_SO        = -shared -Wl,-export-all-symbols
  CPPSTD           =
  MODELSIMBINDIR   = win32aloem
  WLIB             = -lWs2_32
endif

CC                 = gcc
C++                = g++
CFLAGS             = -fPIC                                 \
                     -m32                                  \
                     -g                                    \
                     ${USRFLAGS}                           \
                     -I${SRCDIR}                           \
                     -I${USRCDIR}                          \
                     -I${MODEL_TECH}/../include            \
                     -DVP_MAX_NODES=${MAX_NUM_VPROC}       \
                     -DMODELSIM                            \
                     -D_REENTRANT

# Comman flags for vsim
VSIMFLAGS = -pli ${VPROC_PLI} ${VPROC_TOP}

#------------------------------------------------------
# BUILD RULES
#------------------------------------------------------

all: ${VPROC_PLI}

${VOBJDIR}/%.o: ${SRCDIR}/%.c ${SRC_INCL}
	@${CC} -c ${CFLAGS} $< -o $@

${VOBJDIR}/%.o: ${SRCDIR}/%.cpp ${SRC_INCL}
	@${C++} -c ${CFLAGS} $< -o $@

${VOBJDIR}/%.o: ${USRCDIR}/%.c ${USER_INCL}
	@${CC} -Wno-write-strings -c ${CFLAGS} $< -o $@

${VOBJDIR}/%.o: ${USRCDIR}/%.cpp ${USER_INCL}
	@${C++} ${CPPSTD} -Wno-write-strings -c ${CFLAGS} $< -o $@

${VLIB} : ${VOBJS} ${VOBJDIR}
	@ar cr ${VLIB} ${VOBJS}

${VOBJS}: | ${VOBJDIR}

${VOBJDIR}:
	@mkdir ${VOBJDIR}

${OPDIR}:
	@mkdir ${OPDIR}

${VPROC_PLI}: ${OPDIR} ${VLIB}
	@${C++} ${CPPSTD}                                   \
            ${CFLAGS_SO}                                \
            -Wl,-whole-archive                          \
            ${CFLAGS}                                   \
            -lpthread                                   \
            -L${MODEL_TECH}                             \
            -lmtipli                                    \
            -L${TESTDIR} -lvproc                        \
            -Wl,-no-whole-archive                       \
            ${WLIB}                                     \
            -o $@

#------------------------------------------------------
# CLEANING RULES
#------------------------------------------------------

clean:
	@rm -rf ${VPROC_PLI} ${VLIB} ${VOBJS} ${VOBJDIR}

