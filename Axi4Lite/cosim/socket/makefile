###################################################################
# Makefile for co-simulation test code in Modelsim
#
# Copyright (c) 2022 Simon Southwell.
#
# This file is part of OSVVM co-simulation features.
#
# This code is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This code is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this code. If not, see <http://www.gnu.org/licenses/>.
#
###################################################################

# $MODEL_TECH environment variable must be set

#
# The following four variables can be overridden by the user on the make command line
#
USRCDIR            = ${CURDIR}
OPDIR              = ${CURDIR}
USRFLAGS           =
COSIMDIR           = ${USRCDIR}/..

#
# Internal variable definitions
#

# PLI shared object output
VPROC_PLI          = ${OPDIR}/VProc.so

# Co-simulation root directory
COSIMSRCDIR        = ${COSIMDIR}/code

# Source for the socket class
SKTSRC             = ${COSIMSRCDIR}/osvvm_cosim_skt.cpp
                   
# Get OS type      
OSTYPE             = $(shell uname)

# Set OS specific variables between Linux and Windows (MinGW)
ifeq (${OSTYPE}, Linux)
  WINLIB           =
  TESTEXE          = osvvm_cosim_skt
else               
  WINLIB           = -lWs2_32
  TESTEXE          = osvvm_cosim_skt.exe
endif

# Get all the C++ and header files in the user source code directory
USER_SRC           = $(wildcard ${USRCDIR}/*.cpp)
USER_HDRS          = $(wildcard ${USRCDIR}/*.h)

# Define the compiler executables
CC                 = gcc
C++                = g++

# Define the common flags for compilation
CFLAGS             = -fPIC                                 \
                     -m32                                  \
                     -g                                    \
                     ${USRFLAGS}                           \
                     -I${COSIMSRCDIR}                      \
                     -I${USRCDIR}                          \
                     -I${MODEL_TECH}/../include            \
                     ${WINLIB}

#------------------------------------------------------
# BUILD RULES
#------------------------------------------------------

all: ${TESTEXE} ${VPROC_PLI}

${TESTEXE} : ${USER_SRC} ${USER_HDRS} ${SKTSRC}  ${SKTSRC:%.cpp=%.h}
	@${C++} ${USER_SRC} ${SKTSRC} ${CFLAGS} -DTEST -o $@

${VPROC_PLI} : ${USER_SRC} ${USER_HDRS}
	@make -C ${COSIMDIR} USRCDIR=${USRCDIR} OPDIR=${OPDIR}

#------------------------------------------------------
# CLEANING RULES
#------------------------------------------------------

clean:
	@rm -rf ${TESTEXE}
	@make -C ${COSIMDIR} USRCDIR=${USRCDIR} OPDIR=${OPDIR} clean

