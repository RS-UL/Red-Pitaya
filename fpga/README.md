# Prerequisites

Install libraries used by ModelSim-Altera software:
```bash
# apt-get install libxft2 libxft2:i386 lib32ncurses5
```

# Directory structure

Initially there was a single FPGA project containing all functionality intended to be used by all applications.
Ideally we would maintain this project by fixing bugs and adding new functionality, but there are limits to
how much functionality can be compiled into an FPGA and developers are not always able to keep backward compatibility.

So there are now multiple FPGA projects, some with generic functionality, some with specific functionality for an application.
Common code for all projects is placed directly into the `fpga` directory. Common code are mostly reusable modules.
Project specific code is placed inside the `fpga/prj/name/` directories and is similarly organized as common code.

|  path           | contents
|-----------------|-------------------------------------------------------------
| `fpga/Makefile` | main Makefile, used to run FPGA related tools
| `fpga/*.tcl`    | TCL scripts to be run inside FPGA tools
| `fpga/archive/` | archive of XZ compressed FPGA bit files
| `fpga/doc/`     | documentation (block diagrams, address space, ...)
| `fpga/brd/`     | board files [Vivado System-Level Design Entry](http://www.xilinx.com/support/documentation/sw_manuals/xilinx2016_2/ug895-vivado-system-level-design-entry.pdf))
| `fpga/ip/`      | third party IP, for now Zynq block diagrams
| `fpga/rtl/`     | Verilog (SystemVerilog) "Register-Transfer Level"
| `fpga/sdc/`     | "Synopsys Design Constraints" contains Xilinx design constraints
| `fpga/sim/`     | simulation scripts
| `fpga/tbn/`     | Verilog (SystemVerilog) "test bench"
| `fpga/dts/`     | device tree source include files
| `fpga/prj/name` | project `name` specific code
|                 |
| `fpga/hsi/`     | "Hardware Software Interface" contains FSBL (First Stage Boot Loader) and DTS (Design Tree) builds

# Build process

Xilinx Vivado 2016.2 (including SDK) is required. If installed at the default location, then the next command will properly configure system variables:
```bash
. /opt/Xilinx/Vivado/2016.2/settings64.sh
```

The default mode for building the FPGA is to run a TCL script inside Vivado. Non project mode is used, to avoid the generation of project files, which are too many and difficult to handle. This allows us to only place source files and scripts under version control.

The next scripts perform various tasks:

| TCL script                      | action
|---------------------------------|---------------------------------------------
| `red_pitaya_vivado.tcl`         | creates the bitstream and reports
| `red_pitaya_vivado_project.tcl` | creates a Vivado project for graphical editing
| `red_pitaya_hsi_fsbl.tcl`       | creates FSBL executable binary
| `red_pitaya_hsi_dts.tcl`        | creates device tree sources

To generate a bit file, reports, device tree and FSBL, run (replace `name` with project name):
```bash
make PRJ=name
```

To generate and open a Vivado project using GUI, run:
```bash
make project PRJ=name
```

# Simulation

ModelSim as provided for free from Altera is used to run simulations. Scripts expect the default install location. On Ubuntu the inslall process fails to create an appropriate path to executable files, so this path must be created:
```bash
ln -s $HOME/altera/16.0/modelsim_ase/linux $HOME/altera/16.0/modelsim_ase/linux_rh60
```

To run simulation, Vivado tools have to be installed. There is no need to source `settings.sh`, For now the path to the ModelSim simulator is hard coded into the simulation `Makefile`.
```bash
cd fpga/sim
```

Simulations can be run by running `make` with the bench file name as target:
```bash
make top_tb
```

Some simulations have a waveform window configuration script like `top_tb.tcl` which will prepare an organized waveform window.
```bash
make top_tb WAV=1
```

# Device tree

Device tree is used by Linux to describe features and address space of memory mapped hardware attached to the CPU.

Running `make` inside this directory will create a device tree source and some include files:

| device tree file | contents
|------------------|------------------------------------------------------------
| `zynq-7000.dtsi` | description of peripherals inside PS (processing system)
| `pl.dtsi`        | description of AXI attached peripherals inside PL (programmable logic)
| `system.dts`     | description of all peripherals, includes the above `*.dtsi` files

To enable some Linux drivers (Ethernet, XADC, I2C EEPROM, SPI, GPIO and LED)
additional configuration files. Generic device tree files can be found in `fpga/dts`
while project specific code is in `fpga/prj/name/dts/`

# Signal mapping

## XADC inputs

XADC input data can be accessed through the Linux IIO (Industrial IO) driver interface.

| E2 con | schematic | ZYNQ p/n | XADC in | IIO filename     | measurement target | range |
|--------|-----------|----------|---------|------------------|--------------------|-------|
| AI0    | AIF[PN]0  | B19/A20  | AD8     | in_voltage11_raw | general purpose    | 7.01V |
| AI1    | AIF[PN]1  | C20/B20  | AD0     | in_voltage9_raw  | general purpose    | 7.01V |
| AI2    | AIF[PN]2  | E17/D18  | AD1     | in_voltage10_raw | general purpose    | 7.01V |
| AI3    | AIF[PN]3  | E18/E19  | AD9     | in_voltage12_raw | general purpose    | 7.01V |
|        | AIF[PN]4  | K9 /L10  | AD      | in_voltage0_raw  | 5V power supply    | 12.2V |

### Input range

The default mounting intends for unipolar XADC inputs, which allow for observing only positive signals with a saturation range of *0V ~ 1V*. There are additional voltage dividers use to extend this range up to the power supply voltage. It is possible to configure XADC inputs into a bipolar mode with a range of *-0.5V ~ +0.5V*, but it requires removing R273 and providing a *0.5V ~ 1V* common voltage on the E2 connector.

**NOTE:** Unfortunately there is a design error, where the XADC input range in unipolar mode was thought to be *0V ~ 0.5V*. Consequently the voltage dividers were miss designed for a range of double the supply voltage.

#### 5V power supply

```
                         -------------------0  Vout
           ------------  |  ------------
 Vin  0----| 56.0kOHM |-----| 4.99kOHM |----0  GND
           ------------     ------------
```
Ratio: 4.99/(56.0+4.99)=0.0818
Range: 1V / ratio = 12.2V

#### General purpose inputs

```
                         -------------------0  Vout
           ------------  |  ------------
 Vin  0----| 30.0kOHM |-----| 4.99kOHM |----0  GND
           ------------     ------------
```
Ratio: 4.99/(30.0+4.99)=0.143
Range: 1V / ratio = 7.01


## GPIO LEDs

| LED     | color  | SW driver       | dedicated meaning
|---------|--------|-----------------|----------------------------------
| `[7:0]` | yellow | RP API          | user defined
| `  [8]` | yellow | kernel `MIO[0]` | CPU heartbeat (user defined)
| `  [9]` | reg    | kernel `MIO[7]` | SD card access (user defined)
| ` [10]` | green  | none            | "Power Good" status
| ` [11]` | blue   | none            | FPGA programming "DONE"

For now only LED8 and LED9 are accessible using a kernel driver. LED [7:0] are not driven by a kernel driver, since the Linux GPIO/LED subsystem does not allow access to multiple pins simultaneously.

### Linux access to GPIO/LED

This document is used as reference: http://www.wiki.xilinx.com/Linux+GPIO+Driver

There are 54+64=118 GPIO provided by ZYNQ PS, MIO provides 54 GPIO,
and EMIO provide additional 64 GPIO.

The next formula is used to calculate the `gpio_base` index.
```
base_gpio = ZYNQ_GPIO_NR_GPIOS - ARCH_NR_GPIOS = 1024 - 118 = -906
```

Values for the used macros can be found in the kernel sources.
```bash
$ grep ZYNQ_GPIO_NR_GPIOS drivers/gpio/gpio-zynq.c
#define	ZYNQ_GPIO_NR_GPIOS	118
$ grep -r CONFIG_ARCH_NR_GPIO tmp/linux-xlnx-xilinx-v2016.1
tmp/linux-xlnx-xilinx-v2016.1/.config:CONFIG_ARCH_NR_GPIO=1024
```

Another way to find the `gpio_base` index is to check the given name inside `sysfs`.
```bash
# find /sys/class/gpio/ -name gpiochip*
/sys/class/gpio/gpiochip906
```

The default pin assignment for GPIO is described in the next table.

| FPGA | connector | GPIO             | MIO/EMIO index | `sysfs` index              |color   dedicated meaning     |
|------|-----------|------------------|----------------|----------------------------|------------------------------|
|      |           | `exp_p_io [7:0]` | `EMIO[15: 8]`  | `906+54+[15: 8]=[975:968]` |
|      |           | `exp_n_io [7:0]` | `EMIO[23:16]`  | `906+54+[23:16]=[983:976]` |
|      |           | LED `[7:0]`      | `EMIO[ 7: 0]`  | `906+54+[ 7: 0]=[967:960]` | yellow
|      |           | LED `  [8]`      |  `MIO[ 0]`     | `906+   [ 0]   = 906`      | yellow = CPU heartbeat (user defined)
|      |           | LED `  [9]`      |  `MIO[ 7]`     | `906+   [ 7]   = 913`      | red    = SD card access (user defined)
| `D5` | `E2[ 7]`  | UART1_TX         |  `MIO[ 8]`     | `906+   [ 8]   = 914`      | output only
| `B5` | `E2[ 8]`  | UART1_RX         |  `MIO[ 9]`     | `906+   [ 9]   = 915`      | requires `pinctrl` changes to be active
| `E9` | `E2[ 3]`  | SPI1_MISO        |  `MIO[10]`     | `906+   [10]   = 916`      | requires `pinctrl` changes to be active
| `C6` | `E2[ 4]`  | SPI1_MOSI        |  `MIO[11]`     | `906+   [11]   = 917`      | requires `pinctrl` changes to be active
| `D9` | `E2[ 5]`  | SPI1_SCK         |  `MIO[12]`     | `906+   [12]   = 918`      | requires `pinctrl` changes to be active
| `E8` | `E2[ 6]`  | SPI1_CS#         |  `MIO[13]`     | `906+   [13]   = 919`      | requires `pinctrl` changes to be active
| `B13`| `E2[ 9]`  | I2C0_SCL         |  `MIO[50]`     | `906+   [50]   = 956`      | requires `pinctrl` changes to be active
| `B9` | `E2[10]`  | I2C0_SDA         |  `MIO[51]`     | `906+   [51]   = 957`      | requires `pinctrl` changes to be active

GPIOs are accessible at the `sysfs` index.
The next example will light up LED[0], and read back its value.
```bash
export INDEX=960
echo $INDEX > /sys/class/gpio/export
echo out    > /sys/class/gpio/gpio$INDEX/direction
echo 1      > /sys/class/gpio/gpio$INDEX/value
cat           /sys/class/gpio/gpio$INDEX/value
```

**NOTE**: A new user space ABI for GPIO is coming in kernel v4.8, ioctl will be used instead of `sysfs`.
https://git.kernel.org/cgit/linux/kernel/git/linusw/linux-gpio.git/tree/include/uapi/linux/gpio.h?h=for-next

### Linux access to LED

This document is used as reference: http://www.wiki.xilinx.com/Linux+GPIO+Driver

By providing GPIO/LED details in the device tree, it is possible to access LEDs using a dedicated kernel interface.

To show CPU load on LED 9 use:
```bash
echo heartbeat > /sys/class/leds/led0/trigger
```
To switch LED 8 ON use:
```bash
echo 1 > /sys/class/leds/led0/brightness
```

### PS `pinctrl` for MIO signals

| dts              | description                    |
|------------------|--------------------------------|
| `spi2gpio.dtsi`  | E2 connector, SPI1 signals     |
| `i2c2gpio.dtsi`  | E2 connector, I2C0 signals     |
| `uart2gpio.dtsi` | E2 connector, UART1 signals    |
| `miso2gpio.dtsi` | E2 connector, SPI1 MISO signal |
