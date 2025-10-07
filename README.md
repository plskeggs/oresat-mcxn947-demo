# Oresat MCXN947 Demo

This sample demonstrates use of various hardware modules within the `NXP MCXN947` SoC
mounted on an `Oresat MCXN Breakout` board.

1. Blink an LED forever using the `GPIO API`.
2. Periodically read and display temperature, humidity, and barometric pressure using
an I2C bus connected to a Bosch Sensortec `BME-280` sensor, using the `sensor API`.
3. Generate a continuous 16 bit sawtooth waveform on a DAC output pin using the `DAC driver API`.
4. Periodically monitor two 12 bit ADC channels and display the samples, using the `ADC driver API`.

It also demonstrates:
- Using Kconfig values to control which C source files are included in a build.
- Zephyr logging of error messages, informational messages, and debug messages.
- Setting desired log level on a per-module basis.
- Using printk to display output whether logging is enabled or not.
- Threads.

## Overview

The sample's `main.c` does very little. In fact, all it does is log a message.
Everything else is done in any of the enabled threads.

### Controlling which demo threads are enabled in the build
By default, all 4 threads are enabled, because the source files for them,
`blink.c`, `i2c_sensor.c`, `dac.c`, and `adc.c`, are included in the build.'
This is done using the file `Kconfig` and the file `CMakeLists.txt`.

First, the `Kconfig` file defines symbols for each demo. For example:

```
config BLINK_DEMO
    bool "Blink an LED"
    default y
    help
      Enable the thread which blinks an LED.
```

Note the `default y`. If this line is missing, `default n` is implied.

This affects what is included in the build through conditional compilation
in CMakeLists.txt:

```
# Compile in various demos depending on Kconfig values.
target_sources_ifdef(CONFIG_BLINK_DEMO      app PRIVATE src/blink.c)
target_sources_ifdef(CONFIG_I2C_SENSOR_DEMO app PRIVATE src/i2c_sensor.c)
target_sources_ifdef(CONFIG_DAC_DEMO        app PRIVATE src/dac.c)
target_sources_ifdef(CONFIG_ADC_DEMO        app PRIVATE src/adc.c)
```
Note 1: in `Kconfig` files, the prefix `CONFIG_` is assumed, but not elsewhere.
Note 2: `CONFIG_` symbols are automatically defined in your build folder in
`build/zephyr/include/generated/zephyr/`.

In order to disable a specific demo, the recommended way is via the `prj.conf`
file. This file allows you to set Kconfig symbols to specific values. So to
disable the blink demo, add this line:

```
CONFIG_BLINK_DEMO=n
```

It may be tempting to modify the Kconfig instead to disable demos, or to make
other changes. This should only be done when deciding to make a change that
is the default behavior, rather than a temporary change that can be freely adjusted
later.

### The sample blinks an LED forever using the `GPI API`.

The source code in `blink.c` shows how to:

1. Get a pin specification from the [devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html#dt-guide) as a `gpio_dt_spec`
2. Configure the GPIO pin as an output
3. Toggle the pin forever

See the Zephyr [pwm-blinky](https://docs.zephyrproject.org/latest/samples/basic/blinky_pwm/README.html#pwm-blinky)
sample for a similar sample that uses the `PWM API` instead.

### The sample periodically samples an I2C sensor

The source code in `i2c_sensor.c` shows how to:

1. Connect to a sensor
2. Read a block of samples from it
3. Use a sensor decoder API to decode specific sensor channels

### The sample periodically generates a waveform on a DAC

1. Connect to a DAC
2. Set up the DAC
3. Write waveform values to the DAC

Note: the time between samples generated is not particularly jitter-free.
When timing is critical because the frequency of the generated waveform
is high, other methods like:

 - using a hardware timer to generate interrupts and then update the DAC output
 from the interrupt handler
- generate buffers in RAM of sample data and use DMA to the DAC to automatically
 transfer the samples at a rate set by a hardware timer

### The sample periodically samples two ADC channels

1. Connect to the ADC and two channels within it
2. Setup each of the channels
3. Determine the reference voltage for each channel
4. Read a sequence of samples
5. Convert to millivolts based on the reference voltage

## Hardware connections

- LED and series current limit resistor between `P1_13` and +3.3v
- I2C SCL to `P0_17`
- I2C SDA to `P0_16`
- Connect two 4.7K pull up resistors to I2C SCL and I2C SDA; up = +3.3v
- Connect I2C SCL to BME280 board SCK
- Connect I2C SDA to BME280 board SDI
- Connect the BME280 board Vin to Vbus on pin 1 of the CAN connector J3
- Connect the BME280 board GND to breakout board GND
- DAC output is from `P4_2`
- ADC0 channel 1A inputs from `P4_15`
- ADC0 channel 2A inputs from `P4_23`
- Jumper the DAC to ADC0 channel 1A
- Jumper GND or +3.3v to ADC0 channel 2A

## Building and Running

Build and flash as follows:


```
$ west build -p -b mcxn947_protocard/mcxn947/cpu0
$ west flash -r jlink
```

Note: the CMakeLists.txt file sets the `BOARD_ROOT` so that it does not
need to be specified on the command line.

After flashing, the LED starts to blink and messages with the current LED state,
I2C sensor data, DAC cycles, and ADC samples are printed to the console.
If a runtime error occurs, `<err>` logs are generated.

### Example output with all demos enabled

```
[00:00:00.004,000] <dbg> BME280: bme280_chip_init: ID OK
[00:00:00.014,000] <dbg> BME280: bme280_chip_init: "bme280@77" OK
*** Booting Zephyr OS build v4.2.0 ***

[00:00:00.014,000] <inf> oresat_mcxn947_demo: Oresat MCXN947 Breakout Board Demo

[00:00:00.014,000] <inf> oresat_dac_demo: Starting DAC demo
[00:00:00.014,000] <dbg> oresat_dac_demo: handle_dac: Generating sawtooth signal at DAC channel 0.
[00:00:00.014,000] <dbg> oresat_dac_demo: handle_dac: Number of DAC samples per cycle: 4096, sleep time per sample (us): 100
[00:00:00.014,000] <inf> oresat_adc_demo: Starting ADC demo
[00:00:00.014,000] <inf> oresat_adc_demo: Channels: 2, sequence samples: 8, resolution: 12
[00:00:00.014,000] <inf> oresat_adc_demo: Channel: 0, vref_mv: 3300, gain: 9, acq time: 16386, diff: 0, inp_pos: 1, inp_neg: 0
[00:00:00.014,000] <inf> oresat_adc_demo: Channel: 1, vref_mv: 3300, gain: 9, acq time: 16386, diff: 0, inp_pos: 2, inp_neg: 0
[00:00:00.014,000] <inf> oresat_blink_demo: Starting blink demo
[00:00:00.014,000] <dbg> oresat_blink_demo: handle_blink: The light is blinking!
[00:00:00.014,000] <inf> oresat_i2c_sensor_demo: Starting I2C sensor demo
[00:00:00.014,000] <inf> oresat_i2c_sensor_demo: Found device "bme280@77", getting sensor data
temp: 25.509979; press: 101.703125; humidity: 45.500976
 413, 410, 414, 410, 412, 411, 412, 410,
   0,   0,   0,   0,   0,   0,   0,   0,
 817, 816, 820, 820, 818, 817, 817, 817,
   0,   0,   0,   0,   0,   0,   0,   0,
 1222, 1227, 1223, 1227, 1222, 1222, 1224, 1225,
   0,   0,   0,   0,   0,   0,   0,   0,
 1632, 1633, 1633, 1633, 1632, 1633, 1633, 1633,
   0,   0,   0,   0,   0,   0,   0,   0,
 2042, 2041, 2039, 2042, 2042, 2042, 2042, 2042,
   0,   0,   0,   0,   0,   0,   0,   0,
 2448, 2448, 2450, 2450, 2448, 2451, 2447, 2448,
   0,   0,   0,   0,   0,   0,   0,   0,
 2854, 2853, 2853, 2854, 2857, 2854, 2854, 2856,
   0,   0,   0,   0,   0,   0,   0,   0,
 3262, 3262, 3259, 3262, 3262, 3259, 3259, 3262,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:00.834,000] <inf> oresat_dac_demo: Cycle 1
 349, 348, 344, 346, 344, 347, 348, 347,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:01.014,000] <dbg> oresat_blink_demo: handle_blink: The light is blinking!
 754, 750, 754, 751, 755, 752, 754, 750,
   0,   0,   0,   0,   0,   0,   0,   0,
 1159, 1160, 1159, 1159, 1161, 1159, 1160, 1159,
   0,   0,   0,   0,   0,   0,   0,   0,
 1568, 1568, 1568, 1570, 1567, 1566, 1570, 1568,
   0,   0,   0,   0,   0,   0,   0,   0,
 1977, 1976, 1977, 1979, 1973, 1979, 1977, 1977,
   0,   0,   0,   0,   0,   0,   0,   0,
 2385, 2385, 2385, 2383, 2387, 2385, 2384, 2383,
   0,   0,   0,   0,   0,   0,   0,   0,
 2789, 2788, 2791, 2790, 2791, 2789, 2789, 2792,
   0,   0,   0,   0,   0,   0,   0,   0,
 3196, 3198, 3194, 3198, 3194, 3196, 3194, 3200,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:01.653,000] <inf> oresat_dac_demo: Cycle 2
 282, 277, 281, 280, 281, 281, 281, 283,
   0,   0,   0,   0,   0,   0,   0,   0,
 689, 685, 688, 689, 688, 688, 687, 688,
   0,   0,   0,   0,   0,   0,   0,   0,
 1094, 1094, 1096, 1094, 1095, 1094, 1092, 1094,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:02.015,000] <dbg> oresat_blink_demo: handle_blink: The light is blinking!
 1503, 1498, 1502, 1506, 1503, 1505, 1505, 1503,
   0,   0,   0,   0,   0,   0,   0,   0,
 1911, 1911, 1913, 1911, 1911, 1915, 1911, 1910,
   0,   0,   0,   0,   0,   0,   0,   0,
 2319, 2317, 2317, 2320, 2321, 2321, 2320, 2321,
   0,   0,   0,   0,   0,   0,   0,   0,
 2723, 2723, 2726, 2723, 2726, 2729, 2726, 2729,
   0,   0,   0,   0,   0,   0,   0,   0,
 3132, 3132, 3129, 3132, 3132, 3132, 3129, 3134,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:02.472,000] <inf> oresat_dac_demo: Cycle 3
 215, 216, 217, 219, 217, 216, 215, 215,
   0,   0,   0,   0,   0,   0,   0,   0,
 620, 620, 620, 622, 624, 623, 624, 622,
   0,   0,   0,   0,   0,   0,   0,   0,
 1033, 1032, 1031, 1029, 1027, 1028, 1026, 1030,
   0,   0,   0,   0,   0,   0,   0,   0,
 1435, 1434, 1435, 1436, 1438, 1439, 1439, 1437,
   0,   0,   0,   0,   0,   0,   0,   0,
 1846, 1843, 1849, 1845, 1848, 1848, 1846, 1846,
   0,   0,   0,   0,   0,   0,   0,   0,
[00:00:03.015,000] <dbg> oresat_blink_demo: handle_blink: The light is blinking!
 2254, 2255, 2255, 2255, 2255, 2258, 2252, 2255,
   0,   0,   0,   0,   0,   0,   0,   0,
 2660, 2661, 2661, 2661, 2660, 2661, 2661, 2661,
   0,   0,   0,   0,   0,   0,   0,   0,
 3067, 3067, 3067, 3067, 3067, 3067, 3067, 3067,
   0,   0,   0,   0,   0,   0,   0,   0,
```

## Build errors

You will see a build error at the source code line defining the ``struct
gpio_dt_spec led`` variable if you try to build Blinky for an unsupported
board.

On GCC-based toolchains, the error looks like this:

   `'__device_dts_ord_DT_N_ALIAS_led_P_gpios_IDX_0_PH_ORD' undeclared here (not in a function)`


## Board support

### LED device tree settings

To add support for your board, add something like this to your devicetree's
`.dtsi` file:

```
   / {
   	   aliases {
   	      led0 = &myled0;
   	   };

   	   leds {
   	     compatible = "gpio-leds";
   	     myled0: led_0 {
   			gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
         };
   	   };
   };
```

The above sets your board's `led0` alias to use pin 13 on GPIO controller
`gpio0`. The pin flags `GPIO_ACTIVE_HIGH` mean the LED is on when
the pin is set to its high state, and off when the pin is in its low state.

### I2C bus and BME280 device tree settings

Add this to your board's `.dtsi` file to enable I2C on flexcomm0 and within
that bus, a bme280 slave device at I2C address 0x77:

```
&flexcomm0_lpi2c0 {
	pinctrl-0 = <&pinmux_flexcomm0_lpi2c>;
	pinctrl-names = "default";
	clock-frequency = <I2C_BITRATE_STANDARD>;

	bme280: bme280@77 {
		compatible = "bosch,bme280";
		reg = <0x77>;
        status = "okay";
	};
};

```

And add this to the board's `*-pinctrl.dtsi` file to specify
which pins to use for SDA and SCL:

```
&pinctrl {
...
	pinmux_flexcomm0_lpi2c: pinmux_flexcomm0_lpi2c {
		group0 {
			pinmux = <FC0_P0_PIO0_16>,
    			     <FC0_P1_PIO0_17>;
			slew-rate = "fast";
			drive-strength = "low";
			input-enable;
			bias-pull-up;
			drive-open-drain;
		};
	};

```

### DAC device tree settings

Add this to your board's `.dtsi` file to enable DAC0:

```
/ {
  zephyr,user {
    dac = <&dac0>;
    dac-channel-id = <0>;
    dac-resolution = <12>;
    status = "okay";
};

&dac0 {
	status = "okay";
};
```

And add this to the board's `*-pinctrl.dtsi` file to specify
which pin to output DAC0 on:

```
&pinctrl {
...
  pinmux_dac0: pinmux_dac0 {
    group0 {
        pinmux = <DAC0_OUT_PIO4_2>;
        drive-strength = "low";
        slew-rate = "fast";
    };
  };
```

### ADC device tree settings

Add this to your board's `.dtsi` file to enable ADC0 channels 1A and 2A:

```
...
#include <zephyr/dt-bindings/adc/adc.h>
#include <zephyr/dt-bindings/adc/mcux-lpadc.h>
...

  aliases{
...
    adc0 = &lpadc0;
  }
...
&lpadc0 {
	#address-cells = <1>;
	#size-cells = <0>;

	lpadc0_0: channel@0 {
		reg = <0>;
		zephyr,gain = "ADC_GAIN_1";
		zephyr,reference = "ADC_REF_EXTERNAL0";
		zephyr,vref-mv = <3300>;
		zephyr,acquisition-time = <ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS,2)>;
		zephyr,resolution = <12>;
        zephyr,oversampling = <5>;
		zephyr,input-positive = <MCUX_LPADC_CH1A>;
        /delete-property/ zephyr,differential;
	};
	lpadc0_1: channel@1 {
		reg = <1>;
		zephyr,gain = "ADC_GAIN_1";
		zephyr,reference = "ADC_REF_EXTERNAL0";
		zephyr,vref-mv = <3300>;
		zephyr,acquisition-time = <ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS,2)>;
		zephyr,resolution = <12>;
        zephyr,oversampling = <5>;
		zephyr,input-positive = <MCUX_LPADC_CH2A>;
        /delete-property/ zephyr,differential;
	};
};
```

And add this to your board's `*-pinctrl.dtsi` file to specify which pins
to use for these channels:
 
```
pinmux_lpadc0: pinmux_lpadc0 {
    group0 {
        pinmux = <ADC0_A1_PIO4_15>,
                 <ADC0_A2_PIO4_23>;
        slew-rate = "fast";
        drive-strength = "low";
    };
};

``` 

## Tips

- Examine `build/zephyr/zephyr.dts` to see the final fully combined device tree

- Use the [NXP MCUXpresso Config Tool](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-config-tools-pins-clocks-and-peripherals:MCUXpresso-Config-Tools)
program to explore what each desired hardware module can be multiplexed to which
SoC pins, and to generate example device tree pin control file contents

- Use the [Nordic nRF Connect extension for Microsoft Visual Studio Code](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-VS-Code);
this extension provides a Kconfig tool and a Device Tree tool which can parse the
build output from your project and display helpful errors about potential errors,
view specific settings for each device tree node, and view the SoC pinout

- See (gpio-leds)[https://docs.zephyrproject.org/latest/build/dts/api/bindings/led/gpio-leds.html#std-dtcompatible-gpio-leds]
for more information on defining GPIO-based LEDs in devicetree.

- If you're not sure what to do, check the devicetrees for supported boards which
use the same SoC as your target. See [get-devicetree-outputs](https://docs.zephyrproject.org/latest/build/dts/howtos.html#get-devicetree-outputs) for details.

- See (include/zephyr/dt-bindings/gpio/gpio.h)[https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/dt-bindings/gpio/gpio.h]
for the flags you can use in devicetree.

- If the LED is built in to your board hardware, the alias should be defined in
  your `BOARD.dts` file. Otherwise, you can define one in a [devicetree overlay](https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays).
