#  File Name:         mk_vproc.tcl
#  Purpose:           Scripts for compiling cosimulation shared object
#  Revision:          OSVVM MODELS STANDARD VERSION
#
#  Maintainer:        Simon Southwell email:  simon@gmail.com
#  Contributor(s):
#     Simon Southwell     email:  simon.southwell@gmail.com
#
#  Description
#    Tcl procedures supporting compiling of Co-cimuation C/C++ code
#
#  Developed by:
#        SynthWorks Design Inc.
#        VHDL Training Classes
#        OSVVM Methodology and Model Library
#        11898 SW 128th Ave.  Tigard, Or  97223
#        http://www.SynthWorks.com
#
#  Revision History:
#    Date      Version    Description
#    010/2022   2022.10    Initial version
#
#
#  This file is part of OSVVM.
#
#  Copyright (c) 2022 by SynthWorks Design Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# -------------------------------------------------
# mk_vproc_clean
#   Do a make clean on the VProc test directory
#
proc mk_vproc_clean {srcrootdir testname opdir} {
  exec make -C $srcrootdir USRCDIR=$testname OPDIR=$opdir clean
}

# -------------------------------------------------
# mk_vproc
#   Do a clean make compile for the specified VProc
#   test directory
#
proc mk_vproc {srcrootdir testname opdir} {

  mk_vproc_clean $srcrootdir $testname $opdir
  exec make -C $srcrootdir USRCDIR=$testname OPDIR=$opdir
  file copy -force $srcrootdir/VProc.so $opdir
}


# -------------------------------------------------
# mk_vproc_noclean
#   Do a make compile for the VProc test directory
#   without a clean
#
proc mk_vproc_noclean {srcrootdir testname opdir} {

  exec make -C $srcrootdir USRCDIR=$testname OPDIR=$opdir
  file copy -force $srcrootdir/VProc.so $opdir
}
