#ifndef __HC4067_H__
#define __HC4067_H__

#if __has_include(<zephyr/kernel.h>)
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#define HC4067_HAS_ZEPHYR 1
#else
#include <cstdint>
struct gpio_dt_spec { void *port; unsigned int pin; unsigned int dt_flags; };
using gpio_flags_t = unsigned int;
#endif

class HC4067 {
public:
	HC4067(const gpio_dt_spec &select0,
	       const gpio_dt_spec &select1,
	       const gpio_dt_spec &select2,
	       const gpio_dt_spec &select3,
	       const gpio_dt_spec &enablePin);

	bool initialize();
	void set_enabled(bool enabled);
	int select_channel(uint8_t channel);

	/* Optional ADC integration (Zephyr only). The ADC channel is the MCU ADC
	 * channel wired to the mux SIG pin. Resolution is in bits (e.g., 12). */
	bool configure_adc(const struct device *adc_device,
	                  uint8_t adc_channel_id,
	                  uint8_t resolution_bits = 12);
	int read_channel_adc(uint8_t channel, int16_t &out_sample);

	bool is_initialized() const { return initialized; }
	bool is_enabled() const { return enabled; }
	uint8_t current_channel() const { return activeChannel; }

private:
	gpio_dt_spec s0;
	gpio_dt_spec s1;
	gpio_dt_spec s2;
	gpio_dt_spec s3;
	gpio_dt_spec en;

	bool initialized = false;
	bool enabled = false;
	uint8_t activeChannel = 0;

#ifdef HC4067_HAS_ZEPHYR
	const struct device *adcDev = nullptr;
	uint8_t adcChannelId = 0;
	uint8_t adcResolutionBits = 12;
#endif

	static int configure_output_pin(const gpio_dt_spec &spec, gpio_flags_t extra_flags = 0);
};

#endif // __HC4067_H__


