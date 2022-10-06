--
--  File Name:         OssvmTestCoSimPkg.vhd
--  Design Unit Name:  OssvmTestCoSimPkg
--  Revision:          OSVVM MODELS STANDARD VERSION
--
--  Maintainer:        Jim Lewis      email:  jim@synthworks.com
--  Contributor(s):
--     Jim Lewis            jim@synthworks.com
--     Simon Southwell      simon.southwell@gmail.com
--
--
--  Description:
--      Defines procedures to support co-simulation
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

library ieee ;
  use ieee.std_logic_1164.all ;
  use ieee.numeric_std.all ;
  use ieee.numeric_std_unsigned.all ;

library OSVVM ;
  context OSVVM.OsvvmContext ;

library osvvm_Axi4 ;
  context osvvm_Axi4.Axi4LiteContext ;

use work.vproc_pkg.all;

package OsvvmTestCoSimPkg is

constant WEbit           : integer := 0 ;
constant RDbit           : integer := 1 ;
constant NodeNum         : integer := 0  ; -- Always use node 0 for now
constant ADDR_WIDTH      : integer := 32 ; -- Only 32 bits supported
constant DATA_WIDTH      : integer := 32 ; -- Only 32 bits supported

  ------------------------------------------------------------
  ------------------------------------------------------------
procedure CoSimAccess (
  signal   ManagerRec      : inout  AddressBusRecType ;
  variable OperationFifo   : inout  osvvm.ScoreboardPkg_slv.ScoreboardPType ;
  variable OpRV            : inout  RandomPType ;
  signal   OperationCount  : out    integer ;
  variable RdData          : inout  std_logic_vector ;
  variable Data            : out    std_logic_vector ;
  variable Address         : out    std_logic_vector ;
  variable RnW             : out    integer
  ) ;

  ------------------------------------------------------------
  ------------------------------------------------------------
procedure CoSimTrans (
  signal   ManagerRec      : inout  AddressBusRecType ;
  variable RdData          : inout  std_logic_vector
  ) ;

end package OsvvmTestCoSimPkg ;

-- /////////////////////////////////////////////////////////////////////////////////////////
-- /////////////////////////////////////////////////////////////////////////////////////////

package body OsvvmTestCoSimPkg is

  ------------------------------------------------------------
  ------------------------------------------------------------

procedure CoSimAccess (
  -- Transaction  interface
  signal   ManagerRec      : inout  AddressBusRecType ;

  -- Test interface
  variable OperationFifo   : inout  osvvm.ScoreboardPkg_slv.ScoreboardPType ;
  variable OpRV            : inout  RandomPType ;
  signal   OperationCount  : out    integer ;

  -- Logging interface
  variable RdData          : inout  std_logic_vector ;
  variable Data            : out    std_logic_vector ;
  variable Address         : out    std_logic_vector ;
  variable RnW             : out    integer
  ) is

  type     OperationType     is (WRITE_OP, READ_OP) ;
  subtype  OperationSlvType  is std_logic_vector(0 downto 0) ;

  constant WRITE_OP_INDEX  : integer := OperationType'pos(WRITE_OP) ;
  constant READ_OP_INDEX   : integer := OperationType'pos(READ_OP) ;

  variable VPDataOut       : integer ;
  variable VPAddr          : integer ;
  variable VPRW            : integer ;
  variable RdDataSamp      : integer ;

  variable OperationSlv    : OperationSlvType;

  begin

    RdDataSamp  := to_integer(signed(RdData));

    -- Call VSched
    VTrans(NodeNum,
           0, -- interrupts
           RdDataSamp,
           VPDataOut,
           VPAddr,
           VPRW) ;

    Address       := std_logic_vector(to_signed(VPAddr,    ADDR_WIDTH)) ;
    Data          := std_logic_vector(to_signed(VPDataOut, DATA_WIDTH)) ;

    if to_unsigned(VPRW, 2)(RDbit)
    then
      RnW         := 1;

      -- If a read operation, generate some random data for the subordinate to return
      Data        := OpRV.RandSlv(size => DATA_WIDTH) ;
    else
      RnW         := 0;
    end if;

    -- Let the subordinate know the details of the access
    OperationSlv  := to_slv(RnW, OperationSlv'length) ;
    OperationFifo.push(OperationSlv & Address & Data) ;

    Increment(OperationCount);

    -- Do the Operation
    if (OperationType'val(RnW) = READ_OP)
    then
      Read(ManagerRec, Address, RdData) ;
    else
      Write(ManagerRec, Address, Data) ;
    end if;

  end procedure CoSimAccess ;

  ------------------------------------------------------------
  ------------------------------------------------------------

procedure CoSimTrans (
  -- Transaction  interface
  signal   ManagerRec      : inout  AddressBusRecType ;
  variable RdData          : inout  std_logic_vector
  ) is

  variable VPDataOut       : integer ;
  variable VPAddr          : integer ;
  variable VPRW            : integer ;
  variable VPDataIn        : integer ;

  variable WrData          : std_logic_vector (DATA_WIDTH-1 downto 0);
  variable Address         : std_logic_vector (ADDR_WIDTH-1 downto 0);
  variable Interrupt       : integer := 0 ;

  begin

    -- Sample the read data from last access, saved in RdData inout port
    VPDataIn  := to_integer(signed(RdData));

    -- Call VTrans to generate a new access
    VTrans(NodeNum, Interrupt, VPDataIn, VPDataOut, VPAddr, VPRW) ;

    -- Convert address and write data to std_logic_vectors
    Address       := std_logic_vector(to_signed(VPAddr,    ADDR_WIDTH)) ;
    WrData        := std_logic_vector(to_signed(VPDataOut, DATA_WIDTH)) ;

    -- Do the operation using the transaction interface
    if to_unsigned(VPRW, 2)(RDbit)
    then
      Read  (ManagerRec, Address, RdData) ;
    else
      Write (ManagerRec, Address, WrData) ;
    end if;

  end procedure CoSimTrans ;

end package body OsvvmTestCoSimPkg ;
