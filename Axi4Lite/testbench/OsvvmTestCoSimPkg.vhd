--
--  File Name:         OssvmTestCoSimPkg.vhd
--  Design Unit Name:  OssvmTestCoSimPkg
--  Revision:          OSVVM MODELS STANDARD VERSION
--
--  Maintainer:        Simon Southwell  email: simon.southwell@gmail.com
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

use work.OsvvmVprocPkg.all;

package OsvvmTestCoSimPkg is

constant WEbit           : integer := 0 ;
constant RDbit           : integer := 1 ;
constant NodeNum         : integer := 0  ; -- Always use node 0 for now
constant ADDR_WIDTH      : integer := 32 ;
constant DATA_WIDTH      : integer := 32 ;
constant ADDR_WIDTH_MAX  : integer := 64 ;
constant DATA_WIDTH_MAX  : integer := 64 ;

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
  signal   ManagerRec      : inout  AddressBusRecType 
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
  variable VPDataOutHi     : integer ;
  variable VPAddr          : integer ;
  variable VPAddrHi        : integer ;
  variable VPRW            : integer ;
  variable RdDataSamp      : integer ;
  variable RdDataSampHi    : integer ;
  
  variable VPDataWidth     : integer ;
  variable VPAddrWidth     : integer ;
  
  variable Interrupt       : integer := 0 ; -- currently unconnected

  variable OperationSlv    : OperationSlvType;

  begin

    RdDataSamp  := to_integer(signed(RdData));

    -- Call VTrans to generate a new access
    VTrans(NodeNum,    Interrupt,
           RdDataSamp, RdDataSampHi,
           VPDataOut,  VPDataOutHi, VPDataWidth,
           VPAddr,     VPAddrHi,    VPAddrWidth,
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
  signal   ManagerRec      : inout  AddressBusRecType 
  ) is

  variable VPDataOut       : integer ;
  variable VPDataOutHi     : integer ;
  variable VPAddr          : integer ;
  variable VPAddrHi        : integer ;
  variable VPRW            : integer ;
  variable VPDataIn        : integer ;
  variable VPDataInHi      : integer ;
  
  variable VPDataWidth     : integer ;
  variable VPAddrWidth     : integer ;

  variable RdData          : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;
  variable WrData          : std_logic_vector (DATA_WIDTH_MAX-1 downto 0) ;
  variable Address         : std_logic_vector (ADDR_WIDTH_MAX-1 downto 0) ;
  variable Interrupt       : integer := 0 ; -- currently unconnected

  begin

    -- RdData won't have persisted from last call, so refetch from ManagerRec
    -- which will have persisted (and is not yet updated)
    RdData       := SafeResize(ManagerRec.DataFromModel, RdData'length) ;
    
    -- Sample the read data from last access, saved in RdData inout port
    if ManagerRec.DataWidth > 32 then
      VPDataIn   := to_integer(signed(RdData(31 downto  0))) ;
      VPDataInHi := to_integer(signed(RdData(RdData'length-1 downto 32))) ;
    else
      VPDataIn   := to_integer(signed(RdData(31 downto 0))) ;
      VPDataInHi := 0;
    end if;

    -- Call VTrans to generate a new access
    VTrans(NodeNum,   Interrupt,
           VPDataIn,  VPDataInHi,
           VPDataOut, VPDataOutHi, VPDataWidth,
           VPAddr,    VPAddrHi,    VPAddrWidth,
           VPRW) ;

    -- Convert address and write data to std_logic_vectors
    Address(31 downto  0) := std_logic_vector(to_signed(VPAddr,      32)) ;
    Address(63 downto 32) := std_logic_vector(to_signed(VPAddrHi,    32)) ;
    
    WrData(31 downto 0 )  := std_logic_vector(to_signed(VPDataOut,   32)) ;
    WrData(63 downto 32)  := std_logic_vector(to_signed(VPDataOutHi, 32)) ;

    -- Do the operation using the transaction interface
    if VPRW /= 0 then
      if to_unsigned(VPRW, 2)(RDbit)
      then
        if VPAddrWidth = 64 then
          case VPDataWidth is
          when 64 => Read  (ManagerRec, Address, RdData) ;
          when 32 => Read  (ManagerRec, Address, RdData(31 downto 0)) ;
          when 16 => Read  (ManagerRec, Address, RdData(15 downto 0)) ;
          when  8 => Read  (ManagerRec, Address, RdData( 7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim read transaction (64 bit arch)");
          end case;
        elsif VPAddrWidth = 32 then
          case VPDataWidth is
          when 32 => Read  (ManagerRec, Address(31 downto 0), RdData(31 downto 0)) ;
          when 16 => Read  (ManagerRec, Address(31 downto 0), RdData(15 downto 0)) ;
          when  8 => Read  (ManagerRec, Address(31 downto 0), RdData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim read transaction (32 bit arch)");
          end case ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim read transaction");
        end if;
      else
        if VPAddrWidth = 64 then
          case VPDataWidth is
          when 64 => Write (ManagerRec, Address, WrData) ;
          when 32 => Write (ManagerRec, Address, WrData(31 downto 0)) ;
          when 16 => Write (ManagerRec, Address, WrData(15 downto 0)) ;
          when  8 => Write (ManagerRec, Address, WrData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim write transaction (64 bit arch)");
          end case ;
        elsif VPAddrWidth = 32 then
          case VPDataWidth is
          when 32 => Write (ManagerRec, Address(31 downto 0), WrData(31 downto 0)) ;
          when 16 => Write (ManagerRec, Address(31 downto 0), WrData(15 downto 0)) ;
          when  8 => Write (ManagerRec, Address(31 downto 0), WrData(7 downto 0)) ;
          when others => AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid data width for co-sim write transaction (32 bit arch)");
          end case ;
        else
          AlertIf(ALERTLOG_DEFAULT_ID, true, "Invalid address width for co-sim write transaction");
        end if;
      end if;
    end if;

  end procedure CoSimTrans ;

end package body OsvvmTestCoSimPkg ;
