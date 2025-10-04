#include "hc4067.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hc4067, CONFIG_LOG_DEFAULT_LEVEL);
#include <errno.h>

HC4067::HC4067(const gpio_dt_spec &select0,
	           const gpio_dt_spec &select1,
	           const gpio_dt_spec &select2,
	           const gpio_dt_spec &select3,
	           const gpio_dt_spec &enablePin)
	: s0(select0), s1(select1), s2(select2), s3(select3), en(enablePin) {}

int HC4067::configure_output_pin(const gpio_dt_spec &spec, gpio_flags_t extra_flags)
{
    if (!device_is_ready(spec.port)) {
        LOG_ERR("GPIO port not ready");
        return -ENODEV;
    }
    int rc = gpio_pin_configure_dt(&spec, GPIO_OUTPUT_INACTIVE | extra_flags);
    if (rc != 0) {
        LOG_ERR("Failed to configure GPIO %d rc=%d", spec.pin, rc);
    }
    return rc;
}

bool HC4067::initialize()
{
	int rc = 0;
	rc |= configure_output_pin(s0);
	rc |= configure_output_pin(s1);
	rc |= configure_output_pin(s2);
	rc |= configure_output_pin(s3);
	rc |= configure_output_pin(en);
	if (rc != 0) {
		return false;
	}

    /* Disable by default (EN high for HC4067 is disable) */
    gpio_pin_set_dt(&en, 1);
	enabled = false;
	activeChannel = 0;
	initialized = true;
	return true;
}

void HC4067::set_enabled(bool doEnable)
{
	if (!initialized) return;
    /* HC4067 enable is active LOW: set 0 to enable, 1 to disable */
    gpio_pin_set_dt(&en, doEnable ? 0 : 1);
	enabled = doEnable;
}

int HC4067::select_channel(uint8_t channel)
{
	if (!initialized) return -EINVAL;
	if (channel > 15) return -EINVAL;

    /* Set S0..S3 according to channel LSB..MSB */
    gpio_pin_set_dt(&s0, (channel >> 0) & 0x1);
    gpio_pin_set_dt(&s1, (channel >> 1) & 0x1);
    gpio_pin_set_dt(&s2, (channel >> 2) & 0x1);
    gpio_pin_set_dt(&s3, (channel >> 3) & 0x1);

	activeChannel = channel;

    /* Allow short settling time for the analog switch */
    k_busy_wait(100); /* settle longer for ADC sampling */
	return 0;
}

bool HC4067::configure_adc(const struct device *adc_device,
                           const adc_channel_cfg &channel_cfg_in,
                           uint8_t resolution_bits)
{
    if (adc_device == nullptr || !device_is_ready(adc_device)) {
        LOG_ERR("ADC device not ready");
        return false;
    }

    adcDev = adc_device;
    adcChannelId = channel_cfg_in.channel_id;
    adcResolutionBits = resolution_bits;

    adc_channel_cfg channel_cfg = channel_cfg_in;

    int rc = adc_channel_setup(adcDev, &channel_cfg);
    if (rc) {
        LOG_ERR("adc_channel_setup failed: %d", rc);
        return false;
    }
    return true;
}

int HC4067::read_channel_adc(uint8_t channel, int16_t &out_sample)
{
    if (!initialized) return -EINVAL;
    if (adcDev == nullptr) return -ENODEV;
    int sel = select_channel(channel);
    if (sel != 0) return sel;

    adc_sequence seq = {};
    seq.channels = BIT(adcChannelId);
    seq.buffer = &out_sample;
    seq.buffer_size = sizeof(out_sample);
    seq.resolution = adcResolutionBits;

    int rc = adc_read(adcDev, &seq);
    return rc;
}


