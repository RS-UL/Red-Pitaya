library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity inputCounter is
	port(input: in std_logic; clk: in std_logic; gate: in std_logic; reset: in std_logic;
			count: buffer unsigned(31 downto 0));
end inputCounter;

architecture inputCounter_behaviour of inputCounter is
	signal inputBuffer: std_logic := '0';
	signal inputAck: std_logic := '0';
begin

	process(input,inputAck)
	begin
		if(inputAck = '1') then
			inputBuffer <= '0';
		elsif(input'event and input = '1' and gate = '1') then
			inputBuffer <= '1';
		end if;
	end process;

	process(clk)
		variable reset_last: std_logic := '0';
		variable edgeDetected: boolean := false;
	begin
		if(clk'event and clk = '1') then
			if(inputAck = '1') then
				inputAck <= '0';
				edgeDetected := true;
			else
				inputAck <= inputBuffer;
				edgeDetected := false;
			end if;
			if(reset = '0') then
				if(reset_last = '1') then
					if(edgeDetected) then
						count <= to_unsigned(1,count'length);
					else
						count <= to_unsigned(0,count'length);
					end if;
				elsif(edgeDetected) then
					count <= count + 1;
				end if;
			end if;
			reset_last := reset;
		end if;
	end process;
	
end inputCounter_behaviour;
