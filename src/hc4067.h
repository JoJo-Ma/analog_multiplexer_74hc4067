#ifndef __HC4067_H__
#define __HC4067_H__

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>

/* Convenience macros to construct an HC4067 instance directly from DT */
#define HC4067_INIT_FROM_DT_NODE(var_name, node_id) \
    HC4067 var_name( \
        GPIO_DT_SPEC_GET(node_id, s0_gpios), \
        GPIO_DT_SPEC_GET(node_id, s1_gpios), \
        GPIO_DT_SPEC_GET(node_id, s2_gpios), \
        GPIO_DT_SPEC_GET(node_id, s3_gpios), \
        GPIO_DT_SPEC_GET(node_id, en_gpios))

#define HC4067_INIT_FROM_DT_NODELABEL(var_name, node_label) \
    HC4067_INIT_FROM_DT_NODE(var_name, DT_NODELABEL(node_label))

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

    /* ADC integration: caller provides full channel config; driver applies it
     * and stores channel id and resolution. */
    bool configure_adc(const struct device *adc_device,
                      const adc_channel_cfg &channel_cfg,
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

	const struct device *adcDev = nullptr;
	uint8_t adcChannelId = 0;
	uint8_t adcResolutionBits = 12;

	static int configure_output_pin(const gpio_dt_spec &spec, gpio_flags_t extra_flags = 0);
};

#endif // __HC4067_H__


