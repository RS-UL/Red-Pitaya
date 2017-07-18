----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 08.11.2016 15:07:07
-- Design Name: 
-- Module Name: odmrCounter - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity odmrCounter is
    port(
		clk_i   		: in std_logic;
		rstn_i          : in std_logic; -- active low
		inputs			: in std_logic_vector(3 downto 0);
-- system bus
		sys_addr        : in std_logic_vector(31 downto 0);
		sys_wdata       : in std_logic_vector(31 downto 0);
		sys_sel         : in std_logic_vector(3 downto 0);
		sys_wen         : in std_logic;
		sys_ren         : in std_logic;
		sys_rdata       : buffer std_logic_vector(31 downto 0);
		sys_err         : out std_logic;
		sys_ack         : out std_logic);
	constant numCounters			: integer := 2;
	constant defaultTriggerInput	: integer := 4;
	constant dna                    : std_logic_vector(31 downto 0) := X"A1B2C3D4";
end odmrCounter;


architecture Behavioral of odmrCounter is

    attribute mark_debug : string;
    
	type counter_instructions is (
        idle,                               --000
        immediateCounting_start,            --001
        immediateCounting_waitForTimeout,   --010
        triggeredCounting_waitForTrigger,   --011
        triggeredCounting_store,            --100
        triggeredCounting_predelay,         --101
        triggeredCounting_prestore,         --110
        triggeredCounting_waitForTimeout    --111
    );
    type control_commands is (none, gotoIdle, reset, countImmediately, countTriggered, trigger);
	subtype int16 is signed(15 downto 0);
    subtype uint16 is unsigned(15 downto 0);
    subtype int32 is signed(31 downto 0);
    subtype uint32 is unsigned(31 downto 0);
    type counter_array_type is array(numCounters-1 downto 0) of uint32;
    subtype triggeredCounting_address_type is unsigned(11 downto 0);
    subtype triggeredCounting_data_type is unsigned(17 downto 0);
    type triggeredCounting_address_array_type is array(numCounters-1 downto 0)
        of triggeredCounting_address_type;
    type triggeredCounting_data_array_type is array(numCounters-1 downto 0)
        of triggeredCounting_data_type;
    
    constant triggeredCounting_address_zero : triggeredCounting_address_type
        := to_unsigned(0,triggeredCounting_address_type'length);

	component inputCounter
		port(input: in std_logic; clk: in std_logic; gate: in std_logic; reset: in std_logic;
			count: buffer uint32);
	end component;
	
	component counterRam	port(
        clka    : in std_logic;
        rsta    : in std_logic;
        ena     : in std_logic;
        wea     : in std_logic_vector(0 downto 0);
        addra   : in std_logic_vector(11 downto 0);
        dina    : in std_logic_vector(17 downto 0);
        douta   : out std_logic_vector(17 downto 0);
        clkb    : in std_logic;
        rstb    : in std_logic;
        enb     : in std_logic;
        web     : in std_logic_vector(0 downto 0);
        addrb   : in std_logic_vector(11 downto 0);
        dinb    : in std_logic_vector(17 downto 0);
        doutb   : out std_logic_vector(17 downto 0));
    end component;


	signal counter_clock	: uint32 := to_unsigned(0,32);
	signal counters			: counter_array_type;
	signal counters_reset	: std_logic_vector(numCounters-1 downto 0)
					:= std_logic_vector(to_unsigned(0,numCounters));
	signal counting_stopped	: std_logic := '1';

	signal timeout							: uint32 := to_unsigned(0,32);
	signal predelay							: uint32 := to_unsigned(0,32);
	signal splitTriggeredCountingBins		: boolean := false;
	signal splitTriggeredCountingBinsIndicator
											: std_logic	:= '0';

	signal triggeredCountingBins			: triggeredCounting_address_type
								:= triggeredCounting_address_zero;
	signal triggeredCountingBinRepetitions	: uint16 := to_unsigned(0,16);
	signal triggeredCountingBinRepetitionCounter
											: uint16 := to_unsigned(0,16);
	signal triggeredCountingTriggerMask		: std_logic_vector(inputs'length-1 downto 0)
								:= std_logic_vector(to_unsigned(2**defaultTriggerInput,inputs'length));
	signal triggeredCountingTriggerInvert	: std_logic_vector(inputs'length-1 downto 0)
								:= std_logic_vector(to_unsigned(0,inputs'length));
	signal triggeredCountingTriggerPolarity	: std_logic := '0';
	
	signal triggeredCountingWorkAddress		: triggeredCounting_address_type;
	signal triggeredCountingWorkDataIn		: triggeredCounting_data_array_type;
	signal triggeredCountingWorkDataOut		: triggeredCounting_data_array_type;
	signal triggeredCountingWorkWriteEnable	: std_logic_vector(numCounters-1 downto 0) 
								:= std_logic_vector(to_unsigned(0,numCounters));
	signal triggeredCountingDumpAddress		: triggeredCounting_address_type;
	signal triggeredCountingDumpDataIn		: triggeredCounting_data_array_type;
	signal triggeredCountingDumpDataOut		: triggeredCounting_data_array_type;
	signal triggeredCountingDumpWriteEnable	: std_logic_vector(numCounters-1 downto 0) 
								:= std_logic_vector(to_unsigned(0,numCounters));
	signal triggeredCountingTrigger			: std_logic;
	signal triggeredCountingTriggerBuffer	: std_logic := '0';
	signal triggeredCountingTriggerAck		: std_logic := '0';

	signal controlCommandSignal				: std_logic;
	signal controlCommandSignalBuffer		: std_logic := '0';
	signal controlCommandSignalAck			: std_logic := '0';

	signal counterInstruction				: counter_instructions := idle;
	signal controlCommand                   : control_commands := none;

	signal counterGate						: std_logic;
    signal counterGatingActivated           : boolean;
    signal debug_clock                      : unsigned(31 downto 0);

	function maskedOr(sig: std_logic_vector; mask: std_logic_vector) return std_logic is
		variable t: std_logic := 'Z';
	begin
		for i in sig'range loop
			t := t or (sig(i) and mask(i));
		end loop;
		return t;
	end function;
	
	function Boolean_to_Std_Logic(x : boolean) return std_logic is
	begin
		if x then
			return '1';
		else
			return '0';
		end if;
	end function;

	function Std_Logic_to_Boolean(x : std_logic) return boolean is
	begin
		return x = '1' or x = 'H';
	end function;
	
	function ZeroPad(x: std_logic_vector; size: integer) return std_logic_vector is
	begin
		if x'length >= size then
			return x;
		else
			return std_logic_vector(to_unsigned(0,size-x'length)) & x;
		end if;
	end function;

	function ZeroPadToFit(x: std_logic_vector; y: std_logic_vector) return std_logic_vector is
	begin
		if x'length = y'length then
			return x;
		elsif x'length < y'length then
			return std_logic_vector(to_unsigned(0,y'length-x'length)) & x;
		else
			report "left operand is bigger than right operand" severity error;
		end if;
	end function;

begin

	counterInsts: for i in counters'right to counters'left generate
	begin
		counterInst: inputCounter port map(
				input	=> inputs(i),
				gate	=> counterGate and not counting_stopped,
				clk     => clk_i,
				reset   => counters_reset(i),
				count   => counters(i));
	end generate counterInsts;
	
    counterRamInsts: for i in counters'right to counters'left generate
        begin 
            counterRamInst: counterRam port map (
                clka            => clk_i,
                ena             => '1',
                rsta            => '0',
                wea             => triggeredCountingWorkWriteEnable(i downto i),
                addra           => std_logic_vector(triggeredCountingWorkAddress),
                dina            => std_logic_vector(triggeredCountingWorkDataIn(i)),
                unsigned(douta) => triggeredCountingWorkDataOut(i),
                clkb            => clk_i,
                enb             => '1',
                rstb            => '0',
                web             => triggeredCountingDumpWriteEnable(i downto i),
                addrb           => std_logic_vector(triggeredCountingDumpAddress),
                dinb            => std_logic_vector(triggeredCountingDumpDataIn(i)),
                unsigned(doutb)  => triggeredCountingDumpDataOut(i));
        end generate counterRamInsts;
        
    triggeredCountingTrigger <= triggeredCountingTriggerPolarity 
                    when unsigned((inputs xor triggeredCountingTriggerInvert)
                                and triggeredCountingTriggerMask)
                            = to_unsigned(0,inputs'length+1)
                    else not triggeredCountingTriggerPolarity;
        
    counterGate <= triggeredCountingTrigger when counterGatingActivated else '1';

	process(triggeredCountingTrigger,triggeredCountingTriggerAck)
	begin
		if(triggeredCountingTriggerAck = '1') then
			triggeredCountingTriggerBuffer <= '0';
		elsif(RISING_EDGE(triggeredCountingTrigger)) then
			triggeredCountingTriggerBuffer <= '1';
		end if;
	end process;
	
	process(controlCommandSignal,controlCommandSignalAck)
	begin
		if(controlCommandSignalAck = '1') then
			controlCommandSignalBuffer <= '0';
		elsif(RISING_EDGE(controlCommandSignal)) then
			controlCommandSignalBuffer <= '1';
		end if;
	end process;

	process(clk_i)
		variable triggeredCountingTriggerEdgeDetected	: boolean 	:= false;
		variable counterInstructionBuffer				: counter_instructions	:= idle;
        variable controlCommandSignalValid : boolean := false;
	begin
        if rising_edge(clk_i) then
            if rstn_i = '0' then
                triggeredCountingTriggerAck <= '0';
                triggeredCountingTriggerEdgeDetected := false;
                controlCommandSignalValid := false;
                counterInstructionBuffer := idle;
                counterInstruction <= idle;
                triggeredCountingWorkAddress <= triggeredCounting_address_zero;
                triggeredCountingBinRepetitionCounter <=
                    to_unsigned(0,triggeredCountingBinRepetitionCounter'length);
                splitTriggeredCountingBinsIndicator <= '0';
                debug_clock <= to_unsigned(0,32);
            else
                if(triggeredCountingTriggerAck = '1') then
                    triggeredCountingTriggerAck <= '0';
                    triggeredCountingTriggerEdgeDetected := true;
                else
                    triggeredCountingTriggerAck <= triggeredCountingTriggerBuffer;
                    triggeredCountingTriggerEdgeDetected := false;
                end if;
                if(controlCommandSignalAck = '1') then
                    controlCommandSignalAck <= '0';
                    controlCommandSignalValid := true;
                else
                    controlCommandSignalAck <= controlCommandSignalBuffer;
                    controlCommandSignalValid := false;
                end if;
                counterInstructionBuffer := counterInstruction;
                debug_clock <= debug_clock + 1;
                if controlCommandSignalValid then
                    case controlCommand is
                    when none =>
                    when gotoIdle =>
                        counterInstructionBuffer := idle;
                    when reset =>
                        triggeredCountingWorkAddress <= triggeredCounting_address_zero;
                        triggeredCountingBinRepetitionCounter <=
                            to_unsigned(0,triggeredCountingBinRepetitionCounter'length);
                        splitTriggeredCountingBinsIndicator 	<= '0';
                        counterInstructionBuffer				:= idle;
                        debug_clock                             <= to_unsigned(0,32);
                    when countImmediately =>
                        counterInstructionBuffer				:= immediateCounting_start;
                    when countTriggered =>
                        counterInstructionBuffer				:= triggeredCounting_waitForTrigger;
                    when trigger =>
                        triggeredCountingTriggerEdgeDetected:= true;
                    when others =>
                    end case;
                end if;
                counterInstruction   <= counterInstructionBuffer;
                case counterInstructionBuffer is
                when idle =>
                    counting_stopped <= '1';
                    for i in counters'right to counters'left loop
                        counters_reset(i) <= '1';
                    end loop;
                    for i in counters'right to counters'left loop					
                        triggeredCountingWorkWriteEnable(i) <= '0';
                    end loop;
                when immediateCounting_start =>
                    counter_clock <= timeout;
                    counting_stopped <= '0';
                    for i in counters'right to counters'left loop
                        counters_reset(i) <= '0';
                    end loop;
                    counterInstruction <= immediateCounting_waitForTimeout;
                when immediateCounting_waitForTimeout =>
                    if(counter_clock = 0) then
                        counting_stopped <= '1';
                        for i in counters'right to counters'left loop
                            counters_reset(i) <= '1';
                        end loop;
                        counterInstruction<= idle;
                    else
                        counter_clock		<= counter_clock - 1;
                    end if;
                when triggeredCounting_waitForTrigger =>
                    if(triggeredCountingTriggerEdgeDetected) then
                        if(predelay/=0) then
                            counter_clock <= predelay - 1;
                            counterInstruction <= triggeredCounting_predelay;
                        else
                            counter_clock <= timeout;
                            counting_stopped <= '0';
                            for i in counters'right to counters'left loop
                                counters_reset(i) <= '0';
                            end loop;
                            counterInstruction <= triggeredCounting_waitForTimeout;
                        end if;
                    end if;
                when triggeredCounting_predelay =>
                    if(counter_clock = 0) then
                        counter_clock <= timeout;
                        counting_stopped <= '0';
                        for i in counters'right to counters'left loop
                            counters_reset(i) <= '0';
                        end loop;
                        counterInstruction <= triggeredCounting_waitForTimeout;
                    else
                        counter_clock <= counter_clock - 1;
                    end if;
                when triggeredCounting_waitForTimeout =>
                    if(counter_clock = 0) then
                        counting_stopped <= '1';
                        for i in counters'right to counters'left loop
                            counters_reset(i) <= '1';
                        end loop;
                        counterInstruction <= triggeredCounting_prestore;
                    else
                        counter_clock <= counter_clock - 1;
                    end if;
                when triggeredCounting_prestore =>
                    if splitTriggeredCountingBins then
                        if splitTriggeredCountingBinsIndicator = '1' then
                            for i in counters'right to counters'left loop
                                triggeredCountingWorkDataIn(i) <= 
                                    (triggeredCountingWorkDataOut(i)(
                                            triggeredCountingWorkDataOut(i)'length-1 downto
                                            triggeredCountingWorkDataOut(i)'length/2)
                                        + counters(i)(
                                            triggeredCountingWorkDataOut(i)'length/2-1 downto 0))
                                    & triggeredCountingWorkDataOut(i)(
                                        triggeredCountingWorkDataOut(i)'length/2-1 downto 0);
                            end loop;
                        else
                            for i in counters'right to counters'left loop
                                triggeredCountingWorkDataIn(i) <= 
                                    triggeredCountingWorkDataOut(i)(
                                        triggeredCountingWorkDataOut(i)'length-1 downto
                                        triggeredCountingWorkDataOut(i)'length/2)
                                    & (triggeredCountingWorkDataOut(i)(
                                        triggeredCountingWorkDataOut(i)'length/2-1 downto 0)
                                        + counters(i)(
                                            triggeredCountingWorkDataOut(i)'length/2-1 downto 0));
                            end loop;
                        end if;
                    else
                        for i in counters'right to counters'left loop
                            triggeredCountingWorkDataIn(i) <= triggeredCountingWorkDataOut(i)
                                + counters(i)(triggeredCountingWorkDataOut(i)'length-1 downto 0);
                        end loop;
                    end if;
                    for i in counters'right to counters'left loop
                        triggeredCountingWorkWriteEnable(i) <= '1';
                    end loop;
                    counterInstruction <= triggeredCounting_store;
                when triggeredCounting_store =>
                    for i in counters'right to counters'left loop
                        triggeredCountingWorkWriteEnable(i) <= '0';
                    end loop;
                    if splitTriggeredCountingBins and splitTriggeredCountingBinsIndicator = '0' then
                        splitTriggeredCountingBinsIndicator <= '1';
                    else
                        splitTriggeredCountingBinsIndicator <= '0';
                        if(triggeredCountingBinRepetitionCounter = triggeredCountingBinRepetitions) then
                            triggeredCountingBinRepetitionCounter <= 
                                to_unsigned(0,triggeredCountingBinRepetitionCounter'length);
                            if(triggeredCountingWorkAddress = triggeredCountingBins) then
                                triggeredCountingWorkAddress <= triggeredCounting_address_zero;
                            else
                                triggeredCountingWorkAddress <= triggeredCountingWorkAddress + 1;
                            end if;
                        else
                            triggeredCountingBinRepetitionCounter <= triggeredCountingBinRepetitionCounter + 1;
                        end if;
                    end if;
                    counterInstruction <= triggeredCounting_waitForTrigger;
                when others =>
                    counterInstruction <= idle;
                end case;
            end if;
        end if;
	end process;
   
    process(clk_i) -- axi
        variable counterRamReadInProgress : boolean := false;
        variable counterRamId : integer := 0;
    begin
        if rising_edge(clk_i) then
            controlCommandSignal <= '0';
            if rstn_i = '0' then
                sys_ack <= '0';
                sys_err <= '0';
                counterRamReadInProgress := false;
                counterRamId := 0;
                for i in counters'right to counters'left loop					
                    triggeredCountingDumpWriteEnable(i) <= '0';
                end loop;
                timeout <= to_unsigned(0,32);
                predelay <= to_unsigned(0,32);
                splitTriggeredCountingBins <= false;
                counterGatingActivated <= false;
            else
                sys_err <= '0';
                sys_ack <= '0';
                for i in counters'right to counters'left loop					
                    triggeredCountingDumpWriteEnable(i) <= '0';
                end loop;
                if counterRamReadInProgress then
                    counterRamReadInProgress := false;
                    sys_rdata <= std_logic_vector(to_unsigned(0,sys_rdata'length-triggeredCounting_data_type'length))
                        & std_logic_vector(triggeredCountingDumpDataOut(counterRamId));
                    sys_ack <= sys_ren;
                else
                    case sys_addr(19 downto 16) is
                    when X"0" =>
                        if sys_wen = '1' then 
                            sys_ack <= '1';
                            case sys_addr(15 downto 0) is
                            when X"0000" =>
                                case to_integer(unsigned(sys_wdata)) is
                                when 0 =>
                                    controlCommand <= none;
                                when 1 =>
                                    controlCommand <= gotoIdle;                            
                                when 2 =>
                                    controlCommand <= reset;                            
                                when 3 =>
                                    controlCommand <= countImmediately;
                                when 4 =>
                                    controlCommand <= countTriggered;
                                when 5 =>
                                    controlCommand <= trigger;
                                when others =>
                                    controlCommand <= none;
                                end case; 
                                controlCommandSignal <= '1';
                            when X"0004" =>
                                timeout <= unsigned(sys_wdata(timeout'range));
                            when X"0010" =>
                                triggeredCountingBins <= unsigned(sys_wdata(triggeredCountingBins'range));
                            when X"0014" =>
                                triggeredCountingBinRepetitions <=
                                    unsigned(sys_wdata(triggeredCountingBinRepetitions'range));
                            when X"0018" =>
                                predelay <= unsigned(sys_wdata(predelay'range));
                            when X"001C" =>
                                triggeredCountingTriggerMask <=
                                    sys_wdata(triggeredCountingTriggerMask'length-1 downto 0);
                                triggeredCountingTriggerInvert <=
                                    sys_wdata(triggeredCountingTriggerInvert'length-1+8 downto 8);
                                triggeredCountingTriggerPolarity	<= sys_wdata(16);
                                splitTriggeredCountingBins			<= Std_Logic_to_Boolean(sys_wdata(17));
                                counterGatingActivated				<= Std_Logic_to_Boolean(sys_wdata(18));
                            when others =>
                                sys_ack <= '0';
                                sys_err <= '1';
                            end case;
                        elsif sys_ren = '1' then
                            sys_ack <= '1';
                            case sys_addr(15 downto 0) is
                            when X"0000" =>
                                case counterInstruction is
                                    when idle =>
                                        sys_rdata <= X"00000000";
                                    when immediateCounting_start =>
                                        sys_rdata <= X"00000001";
                                    when immediateCounting_waitForTimeout =>
                                        sys_rdata <= X"00000002";
                                    when triggeredCounting_waitForTrigger =>
                                        sys_rdata <= X"00000003";
                                    when triggeredCounting_store =>
                                        sys_rdata <= X"00000004";
                                    when triggeredCounting_predelay =>
                                        sys_rdata <= X"00000005";
                                    when triggeredCounting_prestore =>
                                        sys_rdata <= X"00000006";
                                    when triggeredCounting_waitForTimeout =>
                                        sys_rdata <= X"00000007";
                                end case;
                            when X"0004" =>
                                sys_rdata <= ZeroPadToFit(std_logic_vector(timeout),sys_rdata); 
                            when X"0008" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(counters(0)),sys_rdata);
                            when X"000C" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(counters(1)),sys_rdata);
                            when X"0010" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(triggeredCountingBins),sys_rdata);
                            when X"0014" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(triggeredCountingBinRepetitions),sys_rdata);
                            when X"0018" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(predelay),sys_rdata);
                            when X"001C" =>
                                 sys_rdata <= ZeroPadToFit(
                                    Boolean_To_Std_Logic(counterGatingActivated)
                                    & Boolean_To_Std_Logic(splitTriggeredCountingBins)
                                    & triggeredCountingTriggerPolarity
                                    & ZeroPad(triggeredCountingTriggerInvert,8)
                                    & ZeroPad(triggeredCountingTriggerMask,8),sys_rdata);
                            when X"0020" =>
                                 if splitTriggeredCountingBins then
                                     sys_rdata <= ZeroPadToFit(
                                        std_logic_vector(triggeredCountingWorkAddress)
                                        & splitTriggeredCountingBinsIndicator,sys_rdata);
                                 else
                                     sys_rdata <= ZeroPadToFit(std_logic_vector(triggeredCountingWorkAddress),sys_rdata);
                                 end if;
                            when X"0024" =>
                                 sys_rdata <= ZeroPadToFit(std_logic_vector(triggeredCountingBinRepetitionCounter),sys_rdata);
                            when X"0028" =>
                                 sys_rdata <= dna;
                            when X"002C" =>
                                 sys_rdata <= std_logic_vector(debug_clock);
                            when others =>
                                sys_rdata <= X"00000000";
                            end case;
                         end if;
                    when X"1" =>
                        if sys_addr(15 downto 15) = "0" and (sys_ren = '1' or sys_wen = '1') then
                            triggeredCountingDumpAddress <= unsigned(sys_addr(13 downto 2));
                            counterRamId := to_integer(unsigned(sys_addr(14 downto 14)));
                            if sys_wen = '1' then
                                triggeredCountingDumpDataIn(counterRamId) <= unsigned(sys_wdata(triggeredCountingDumpDataIn'range));
                                triggeredCountingDumpWriteEnable(counterRamId) <= '1';
                                 sys_ack <= '1';
                            elsif sys_ren = '1' then
                                counterRamReadInProgress := true;
                            end if;
                        end if;
                    when others =>
                        sys_ack <= sys_wen or sys_ren;
                        sys_rdata <= X"00000000";
                    end case;
                end if; 
            end if;
        end if;
    end process;
end Behavioral;
