#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <errno.h>
#include <zephyr/drivers/uart.h>
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
#include "hc4067.h"
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
LOG_MODULE_REGISTER(app);

using namespace std;

int main_tick_interval = 100;



int main(void)
{
    /* Construct HC4067 directly from Devicetree */
    HC4067_INIT_FROM_DT_NODELABEL(mux, mux0);
    const struct device *adc_dev = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(DT_PATH(zephyr_user)));
    adc_channel_cfg adc_cfg = {};
    adc_cfg.gain = ADC_GAIN_1_4;
    adc_cfg.reference = ADC_REF_INTERNAL;
    adc_cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
    adc_cfg.channel_id = DT_IO_CHANNELS_INPUT(DT_PATH(zephyr_user));
    adc_cfg.differential = 0;
    
    if (!mux.initialize()) {
        LOG_ERR("Failed to initialize HC4067 mux");
        return -EIO;
    }
    mux.set_enabled(true);
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -EIO;
    }
    if (!mux.configure_adc(adc_dev, adc_cfg, 12)) {
        LOG_ERR("Failed to configure ADC in mux driver");
        return -EIO;
    }
    
    LOG_INF("HC4067 initialized and enabled");
    
    while (1) {
        char line[256];
        int16_t sample;
        int ofs = 0;
        for (uint8_t ch = 0; ch < 3; ++ch) {
            int ret = mux.read_channel_adc(ch, sample);
            int value = ret == 0 ? (int)sample : -1;
            ofs += snprintf(&line[ofs], sizeof(line) - ofs, "%d", value);
            if (ch != 15 && ofs < (int)sizeof(line) - 2) {
                line[ofs++] = ' ';
            }
        }
        line[ofs] = '\0';
        LOG_INF("%s", line);
        k_sleep(K_MSEC(main_tick_interval));
    }
    
	return 0;
}

