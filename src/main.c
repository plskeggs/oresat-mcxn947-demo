#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h> 
#include <zephyr/sys/__assert.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_template, LOG_LEVEL_DBG);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* 1000 msec = 1 sec */
#define LED_SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static int handle_blink(void)
{
	int ret;
	const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

	/* Can we use the LED? */
	if (!device_is_ready(led.port)) {
		return -1;
	}

	/* Set it up */
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return ret;
	}

	/* Thread loop */
	while (1) {
		gpio_pin_toggle_dt(&led);
		printk("LED: The light is blinking!\n");
		k_msleep(LED_SLEEP_TIME_MS);
	}
	return 0;
}

#define BME_SLEEP_TIME_MS   5000

static const struct device *check_bme280_device(const struct device *bme_dev)
{
	if (bme_dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		printk("\nError: no device found.\n");
		return NULL;
	}

	if (!device_is_ready(bme_dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       bme_dev->name);
		return NULL;
	}

	printk("Found device \"%s\", getting sensor data\n", bme_dev->name);
	return bme_dev;
}

	/*
	 * Get a device structure from a devicetree node with compatible
	 * "bosch,bme280". (If there are multiple, just pick one.)
	 */
	SENSOR_DT_READ_IODEV(iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
			{SENSOR_CHAN_AMBIENT_TEMP, 0},
			{SENSOR_CHAN_HUMIDITY, 0},
			{SENSOR_CHAN_PRESS, 0});

	RTIO_DEFINE(ctx, 1, 1);

static int handle_bme280(void)
{
	int ret;
	uint8_t buf[128];
	const struct device *const bme_dev = DEVICE_DT_GET_ANY(bosch_bme280);

	/* Can we use the I2C sensor? */
	if (check_bme280_device(bme_dev) == NULL) {
		printk("Could not find the BME280\n");
		return -1;
	}

	/* Thread loop */
	while (1) {
		ret = sensor_read(&iodev, &ctx, buf, 128);
		if (ret != 0) {
			printk("%s: sensor_read() failed: %d\n", bme_dev->name, ret);
			break;
		}

		const struct sensor_decoder_api *decoder;

		ret = sensor_get_decoder(bme_dev, &decoder);
		if (ret != 0) {
			printk("%s: sensor_get_decode() failed: %d\n", bme_dev->name, ret);
			break;
		}

		uint32_t temp_fit = 0;
		struct sensor_q31_data temp_data = {0};

		decoder->decode(buf, (struct sensor_chan_spec) {SENSOR_CHAN_AMBIENT_TEMP, 0},
						&temp_fit, 1, &temp_data);

		uint32_t press_fit = 0;
		struct sensor_q31_data press_data = {0};

		decoder->decode(buf, (struct sensor_chan_spec) {SENSOR_CHAN_PRESS, 0},
						&press_fit, 1, &press_data);

		uint32_t hum_fit = 0;
		struct sensor_q31_data hum_data = {0};

		decoder->decode(buf, (struct sensor_chan_spec) {SENSOR_CHAN_HUMIDITY, 0},
						&hum_fit, 1, &hum_data);

		printk("BME280: temp: %s%d.%d; press: %s%d.%d; humidity: %s%d.%d\n",
			PRIq_arg(temp_data.readings[0].temperature, 6, temp_data.shift),
			PRIq_arg(press_data.readings[0].pressure, 6, press_data.shift),
			PRIq_arg(hum_data.readings[0].humidity, 6, hum_data.shift));

		k_msleep(BME_SLEEP_TIME_MS);
	}
	return ret;
}

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#define DAC_CHANNEL_ID DT_PROP(ZEPHYR_USER_NODE, dac_channel_id)
#define DAC_RESOLUTION DT_PROP(ZEPHYR_USER_NODE, dac_resolution)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif

int handle_dac(void)
{
	int ret;
	const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);
	const struct dac_channel_cfg dac_ch_cfg = {
		.channel_id  = DAC_CHANNEL_ID,
		.resolution  = DAC_RESOLUTION,
	#if defined(CONFIG_DAC_BUFFER_NOT_SUPPORT)
		.buffered = false,
	#else
		.buffered = true,
	#endif /* CONFIG_DAC_BUFFER_NOT_SUPPORT */
	};

	/* Can we use the DAC? */
	if (!device_is_ready(dac_dev)) {
		printk("DAC device %s is not ready\n", dac_dev->name);
		return -1;
	}

	/* Set it up */
	ret = dac_channel_setup(dac_dev, &dac_ch_cfg);
	if (ret != 0) {
		printk("Setting up of DAC channel failed with code %d\n", ret);
		return ret;
	}

	/* Number of valid DAC values, e.g. 4096 for 12-bit DAC */
	const int dac_values = 1U << DAC_RESOLUTION;
	/*
	 * 100 usec sleep leads to about 0.4 sec signal period for 12-bit
	 * DACs. For DACs with lower resolution, sleep time needs to
	 * be increased.
	 */
	const int sleep_time = (4096 / dac_values > 0 ? 4096 / dac_values : 1) * 100;

	printk("DAC: Generating sawtooth signal at DAC channel %d.\n", DAC_CHANNEL_ID);
	printk("DAC: Number of DAC samples per cycle: %d, sleep time per sample (ms): %d\n",
		   dac_values, sleep_time);
	int cycle = 0;

	while (1) {
		for (int i = 0; i < dac_values; i++) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, i);
			if (ret != 0) {
				printk("dac_write_value() failed with code %d\n", ret);
				return ret;
			}
			k_sleep(K_USEC(sleep_time));
		}
		printk("DAC: cycle %u\n", ++cycle);
	}
	return 0;
}

int main(void)
{
	printk("\nOresat MCXN947 Breakout Board Demo\n");
	return 0;
}

K_THREAD_DEFINE(blink_id, STACKSIZE, handle_blink, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(bme280_id, STACKSIZE, handle_bme280, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(dac_id, STACKSIZE, handle_dac, NULL, NULL, NULL, PRIORITY, 0, 0);

