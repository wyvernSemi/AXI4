--
--  File Name:           TbAxi4_CoSim.vhd
--  Design Unit Name:    Architecture of TestCtrl
--  Revision:            OSVVM MODELS STANDARD VERSION
--
--  Maintainer:          Simon Southwell  email: simon.southwell@gmail.com
--  Contributor(s):
--     Simon Southwell  simon.southwell@gmail.com
--
--
--  Description:
--      Test transaction source
--
--
--  Developed by:
--        SynthWorks Design Inc.
--        VHDL Training Classes
--        http://www.SynthWorks.com
--
--  Revision History:
--    Date      Version    Description
--    09/2022   2022       Initial revision
--
--
--  This file is part of OSVVM.
--
--  Copyright (c) 2022 by SynthWorks Design Inc.
--
--  Licensed under the Apache License, Version 2.0 (the "License");
--  you may not use this file except in compliance with the License.
--  You may obtain a copy of the License at
--
--      https://www.apache.org/licenses/LICENSE-2.0
--
--  Unless required by applicable law or agreed to in writing, software
--  distributed under the License is distributed on an "AS IS" BASIS,
--  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--  See the License for the specific language governing permissions and
--  limitations under the License.
--

library osvvm_TbAxi4Lite ;

use osvvm_TbAxi4Lite.OsvvmVprocPkg.all;
use osvvm_TbAxi4Lite.OsvvmTestCoSimPkg.all;

library osvvm_axi4 ;

  use osvvm_axi4.Axi4OptionsPkg.all ;
  use osvvm_axi4.Axi4ModelPkg.all ;
  use osvvm_axi4.Axi4InterfaceCommonPkg.all ;
  use osvvm_axi4.Axi4LiteInterfacePkg.all ;
  use osvvm_axi4.Axi4CommonPkg.all ;

architecture CoSim of TestCtrl is
    
  signal TestDone : integer_barrier := 1 ;
  signal Node     : integer := 0 ;

  signal TestActive : boolean := TRUE ;

  signal OperationCount : integer := 0 ;

begin

  ------------------------------------------------------------
  -- ControlProc
  --   Set up AlertLog and wait for end of test
  ------------------------------------------------------------
  ControlProc : process
  begin
    -- Initialization of test
    SetAlertLogName("TbAxi4_CoSim") ;
    SetLogEnable(PASSED, TRUE) ;    -- Enable PASSED logs
    SetLogEnable(INFO, TRUE) ;    -- Enable INFO logs

    -- Wait for testbench initialization
    wait for 0 ns ;  wait for 0 ns ;
    TranscriptOpen(OSVVM_RESULTS_DIR & "TbAxi4_CoSim.txt") ;
    SetTranscriptMirror(TRUE) ;

    -- Wait for Design Reset
    wait until nReset = '1' ;
    ClearAlerts ;

    -- Wait for test to finish
    WaitForBarrier(TestDone, 1 ms) ;
    AlertIf(now >= 1 ms, "Test finished due to timeout") ;
    AlertIf(GetAffirmCount < 1, "Test is not Self-Checking");

    TranscriptClose ;
    -- Printing differs in different simulators due to differences in process order execution
    -- AlertIfDiff("./results/TbAxi4_CoSim.txt", "../sim_shared/validated_results/TbAxi4_CoSim.txt", "") ;

    EndOfTestReports ;
    std.env.stop ;
    wait ;
  end process ControlProc ;


  ------------------------------------------------------------
  -- ManagerProc
  --   Generate transactions for AxiManager
  ------------------------------------------------------------
  ManagerProc : process
    variable OpRV           : RandomPType ;
    variable WaitForClockRV : RandomPType ;
    variable counts         : integer;

    -- CoSim variables
    variable RnW            : integer ;
    variable Ticks          : integer ;

  begin
    -- Initialize Randomization Objects
    OpRV.InitSeed(OpRv'instance_name) ;
    WaitForClockRV.InitSeed(WaitForClockRV'instance_name) ;

    -- Initialise VProc code
    Vinit(0);

    -- Find exit of reset
    wait until nReset = '1' ;
    WaitForClock(ManagerRec, 2) ;

    -- Distribution for Test Operations
    counts := 1000 ;

    OperationLoop : loop

      -- 20 % of the time add a no-op cycle with a delay of 1 to 5 clocks
      if WaitForClockRV.DistInt((8, 2)) = 1 then
        WaitForClock(ManagerRec, WaitForClockRV.RandInt(1, 5)) ;
      end if ;

      -- Call CoSimTrans procedure to generate an access from the running VProc program
      CoSimTrans (ManagerRec);

      -- Update counts
      counts := counts - 1;

      -- Finish when counts == 0
      exit when counts = 0 ;

    end loop OperationLoop ;

    TestActive <= FALSE ;
    -- Allow Subordinate to catch up before signaling OperationCount (needed when WRITE_OP is last)
    -- wait for 0 ns ;  -- this is enough
    WaitForClock(ManagerRec, 2) ;
    Increment(OperationCount) ;

    -- Wait for outputs to propagate and signal TestDone
    WaitForClock(ManagerRec, 2) ;
    WaitForBarrier(TestDone) ;
    wait ;
  end process ManagerProc ;

end CoSim ;

Configuration TbAxi4_CoSim of TbAxi4Cosim is
  for TestHarness
    for TestCtrl_1 : TestCtrl
      use entity work.TestCtrl(CoSim) ;
    end for ;
  end for ;
end TbAxi4_CoSim ;