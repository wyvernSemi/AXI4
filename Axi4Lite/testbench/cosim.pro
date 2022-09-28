#  File Name:               testbench.pro
#  Revision:                OSVVM MODELS STANDARD VERSION
#                           
#  Maintainer:              Jim Lewis      email:  jim@synthworks.com
#  Contributor(s):
#     Jim Lewis             jim@synthworks.com
#     Simon Soutrhwell      simon.southwell@gmail.com
#
#
#  Description:
#        Script to run one Axi4 Lite CoSim test  
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

library    osvvm_TbAxi4Lite

analyze    ../cosim/vproc_pkg.vhd
analyze    OsvvmTestCommonPkg.vhd
analyze    OsvvmTestCoSimPkg.vhd
           
analyze    TestCtrl_e.vhd
analyze    TbAxi4.vhd

analyze    TbAxi4_CoSim.vhd
simulate   TbAxi4_CoSim -pli VProc.so

#RunTest TbAxi4_RandomReadWrite.vhd

