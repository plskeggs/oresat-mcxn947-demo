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

The Blinky sample blinks an LED forever using the `GPI API`.

The source code shows how to:

1. Get a pin specification from the `devicetree` as a `gpio_dt_spec`
2. Configure the GPIO pin as an output
3. Toggle the pin forever

See the Zephyr `pwm-blinky` sample for a similar sample that uses the `PWM API` instead.

## Requirements

Your board must:

1. Have an LED connected via a GPIO pin (these are called "User LEDs" on many of Zephyr's `boards`).
2. Have the LED configured using the `led0` devicetree alias.

## Hardware connections

- LED to `P1_13`
- I2C SCL to `P0_17`
- I2C SDA to `P0_16`
- DAC output from `P4_2`
- ADC0 channel 1A to `P4_15`
- ADC0 channel 2A to `P4_23`
- Jumper the DAC to ADC0 channel 1A
- Jumper GND or +3.3v to ADC0 channel 2A
- Connect two 4.7K pull up resistors to I2C SCL and I2C SDA; up = +3.3v

## Building and Running

Build and flash as follows:

After flashing, the LED starts to blink and messages with the current LED state
are printed on the console. If a runtime error occurs, the sample exits without
printing to the console.

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
[00:00:00.014,000] <inf> app_template: Starting blink demo
[00:00:00.014,000] <dbg> app_template: handle_blink: The light is blinking!
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
[00:00:01.014,000] <dbg> app_template: handle_blink: The light is blinking!
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
[00:00:02.015,000] <dbg> app_template: handle_blink: The light is blinking!
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
[00:00:03.015,000] <dbg> app_template: handle_blink: The light is blinking!
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


## Adding board support

To add support for your board, add something like this to your devicetree:

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

Tips:

- See `gpio-leds` for more information on defining GPIO-based LEDs in devicetree.

- If you're not sure what to do, check the devicetrees for supported boards which
  use the same SoC as your target. See `get-devicetree-outputs` for details.

- See `include/zephyr/dt-bindings/gpio/gpio.h` for the flags you can use in devicetree.

- If the LED is built in to your board hardware, the alias should be defined in
  your `BOARD.dts` file. Otherwise, you can define one in a `devicetree overlay`.
