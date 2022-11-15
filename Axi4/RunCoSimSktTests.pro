#  File Name:         RunCoSimSktTests.pro
#  Revision:          OSVVM MODELS STANDARD VERSION
#
#  Maintainer:        Simon Southwell      email:  simon.southwell@gmail.com
#  Contributor(s):
#     Simon Southwell      simon.southwell@gmail.com
#
#
#  Description:
#        Script to run Axi4 Lite co-sim socket test  
#
#  Developed for:
#        SynthWorks Design Inc.
#        VHDL Training Classes
#        11898 SW 128th Ave.  Tigard, Or  97223
#        http://www.SynthWorks.com
#
#  Revision History:
#    Date      Version    Description
#     10/2022  ????.??    Compile Script for OSVVM co-sim
#
#
#  This file is part of OSVVM.
#  
#  Copyright (c) 2019 - 2020 by SynthWorks Design Inc.  
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
TestSuite  Axi4Full
library    osvvm_TbAxi4

include  ./testbench/testbench.pro
include  ./TestCases/cosimSkt.pro