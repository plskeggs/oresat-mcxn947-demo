#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h> 

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_template, LOG_LEVEL_DBG);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * Get a device structure from a devicetree node with compatible
 * "bosch,bme280". (If there are multiple, just pick one.)
 */
const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);

SENSOR_DT_READ_IODEV(iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
		{SENSOR_CHAN_AMBIENT_TEMP, 0},
		{SENSOR_CHAN_HUMIDITY, 0},
		{SENSOR_CHAN_PRESS, 0});

RTIO_DEFINE(ctx, 1, 1);

static const struct device *check_bme280_device(void)
{
	if (dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		printk("\nError: no device found.\n");
		return NULL;
	}

	if (!device_is_ready(dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       dev->name);
		return NULL;
	}

	printk("Found device \"%s\", getting sensor data\n", dev->name);
	return dev;
}

int main(void)
{
	int ret;
	const struct device *dev = check_bme280_device();

	if (dev == NULL) {
		printk("Could not find the BME280\n");
		return 0;
	}
	if (!device_is_ready(led.port)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (1) {
		gpio_pin_toggle_dt(&led);
		printk("The light is blinking!\n");

		uint8_t buf[128];

		int rc = sensor_read(&iodev, &ctx, buf, 128);

		if (rc != 0) {
			printk("%s: sensor_read() failed: %d\n", dev->name, rc);
			return rc;
		}

		const struct sensor_decoder_api *decoder;

		rc = sensor_get_decoder(dev, &decoder);

		if (rc != 0) {
			printk("%s: sensor_get_decode() failed: %d\n", dev->name, rc);
			return rc;
		}

		uint32_t temp_fit = 0;
		struct sensor_q31_data temp_data = {0};

		decoder->decode(buf,
			(struct sensor_chan_spec) {SENSOR_CHAN_AMBIENT_TEMP, 0},
			&temp_fit, 1, &temp_data);

		uint32_t press_fit = 0;
		struct sensor_q31_data press_data = {0};

		decoder->decode(buf,
				(struct sensor_chan_spec) {SENSOR_CHAN_PRESS, 0},
				&press_fit, 1, &press_data);

		uint32_t hum_fit = 0;
		struct sensor_q31_data hum_data = {0};

		decoder->decode(buf,
				(struct sensor_chan_spec) {SENSOR_CHAN_HUMIDITY, 0},
				&hum_fit, 1, &hum_data);

		printk("temp: %s%d.%d; press: %s%d.%d; humidity: %s%d.%d\n",
			PRIq_arg(temp_data.readings[0].temperature, 6, temp_data.shift),
			PRIq_arg(press_data.readings[0].pressure, 6, press_data.shift),
			PRIq_arg(hum_data.readings[0].humidity, 6, hum_data.shift));

		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
