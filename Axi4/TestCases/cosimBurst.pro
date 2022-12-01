#  File Name:               cosim.pro
#  Revision:                OSVVM MODELS STANDARD VERSION
#
#  Maintainer:              Simon Southwell      simon.southwell@gmail.com
#  Contributor(s):
#     Simon Southwell       simon.southwell@gmail.com
#
#
#  Description:
#        Script to run Axi4 Lite CoSim tests
#
#  Developed for:
#        SynthWorks Design Inc.
#        VHDL Training Classes
#        11898 SW 128th Ave.  Tigard, Or  97223
#        http://www.SynthWorks.com
#
#  Revision History:
#    Date      Version    Description
#     9/2022   2022.01    Initial version
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

source     $::osvvm::CurrentWorkingDirectory/../../../CoSim/Scripts/MakeVproc.tcl

library    osvvm_tbcosim

analyzeForeignProcs
analyze    ../../../CoSim/src/OsvvmTestCoSimPkg.vhd

library    osvvm_TbAxi4
analyze    TbAxi4_CoSim.vhd

simulate   TbAxi4_CoSim [ mk_vproc $::osvvm::CurrentWorkingDirectory/../../../CoSim tests/usercode_burst  ]
