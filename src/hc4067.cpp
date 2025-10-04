#include "hc4067.h"

#if __has_include(<zephyr/logging/log.h>)
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hc4067, CONFIG_LOG_DEFAULT_LEVEL);
#else
#include <cstdio>
#define LOG_ERR(fmt, ...) std::printf("[hc4067][ERR] " fmt "\n", ##__VA_ARGS__)
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENODEV
#define ENODEV 19
#endif
#endif

HC4067::HC4067(const gpio_dt_spec &select0,
	           const gpio_dt_spec &select1,
	           const gpio_dt_spec &select2,
	           const gpio_dt_spec &select3,
	           const gpio_dt_spec &enablePin)
	: s0(select0), s1(select1), s2(select2), s3(select3), en(enablePin) {}

int HC4067::configure_output_pin(const gpio_dt_spec &spec, gpio_flags_t extra_flags)
{
#ifdef HC4067_HAS_ZEPHYR
    if (!device_is_ready(spec.port)) {
        LOG_ERR("GPIO port not ready");
        return -ENODEV;
    }
    int rc = gpio_pin_configure_dt(&spec, GPIO_OUTPUT_INACTIVE | extra_flags);
    if (rc != 0) {
        LOG_ERR("Failed to configure GPIO %d rc=%d", spec.pin, rc);
    }
    return rc;
#else
    (void)spec; (void)extra_flags;
    return 0;
#endif
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
#ifdef HC4067_HAS_ZEPHYR
    gpio_pin_set_dt(&en, 1);
#endif
	enabled = false;
	activeChannel = 0;
	initialized = true;
	return true;
}

void HC4067::set_enabled(bool doEnable)
{
	if (!initialized) return;
	/* HC4067 enable is active LOW: set 0 to enable, 1 to disable */
#ifdef HC4067_HAS_ZEPHYR
    gpio_pin_set_dt(&en, doEnable ? 0 : 1);
#endif
	enabled = doEnable;
}

int HC4067::select_channel(uint8_t channel)
{
	if (!initialized) return -EINVAL;
	if (channel > 15) return -EINVAL;

	/* Set S0..S3 according to channel LSB..MSB */
#ifdef HC4067_HAS_ZEPHYR
    gpio_pin_set_dt(&s0, (channel >> 0) & 0x1);
    gpio_pin_set_dt(&s1, (channel >> 1) & 0x1);
    gpio_pin_set_dt(&s2, (channel >> 2) & 0x1);
    gpio_pin_set_dt(&s3, (channel >> 3) & 0x1);
#endif

	activeChannel = channel;

	/* Allow short settling time for the analog switch */
    #ifdef HC4067_HAS_ZEPHYR
    k_busy_wait(100); /* settle longer for ADC sampling */
    #endif
	return 0;
}

#ifdef HC4067_HAS_ZEPHYR
bool HC4067::configure_adc(const struct device *adc_device,
                           uint8_t adc_channel_id,
                           uint8_t resolution_bits)
{
    if (adc_device == nullptr || !device_is_ready(adc_device)) {
        LOG_ERR("ADC device not ready");
        return false;
    }

    adcDev = adc_device;
    adcChannelId = adc_channel_id;
    adcResolutionBits = resolution_bits;

    adc_channel_cfg channel_cfg = {};
    /* Use ~11 dB attenuation equivalent to cover ~0-3.3V (driver maps gain) */
    /* For ESP32-C3, this gain maps to ~11 dB attenuation (~0-3.3V FS) */
    channel_cfg.gain = ADC_GAIN_1_4;
    channel_cfg.reference = ADC_REF_INTERNAL;
    channel_cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
    channel_cfg.channel_id = adcChannelId;
    channel_cfg.differential = 0;

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
#endif


