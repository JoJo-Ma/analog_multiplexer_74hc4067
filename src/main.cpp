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

bool net_if_ready = false;




struct interface_set_params {
    int interface;
    bool state;
};

// Work item for interface_set
static struct k_work interface_set_work;
static struct interface_set_params interface_params;



/* Remove unused UART reference to avoid warnings */




void interface_set(int interface, bool state);



void interface_set(int interface, bool state)
{
    
}

static void interface_set_work_handler(struct k_work *work)
{
    interface_set(interface_params.interface, interface_params.state);
}



int main(void)
{
    k_work_init(&interface_set_work, interface_set_work_handler);
    
    /* Devicetree-backed GPIO specs for the HC4067 mux */
    const gpio_dt_spec mux_en = GPIO_DT_SPEC_GET(DT_NODELABEL(mux0), en_gpios);
    const gpio_dt_spec mux_s0 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux0), s0_gpios);
    const gpio_dt_spec mux_s1 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux0), s1_gpios);
    const gpio_dt_spec mux_s2 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux0), s2_gpios);
    const gpio_dt_spec mux_s3 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux0), s3_gpios);
    const struct device *adc_dev = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(DT_PATH(zephyr_user)));
    const adc_channel_cfg adc_cfg = {
        .gain = ADC_GAIN_1,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id = DT_IO_CHANNELS_INPUT(DT_PATH(zephyr_user)),
        .differential = 0,
    };
    struct adc_sequence adc_seq = {
        .channels = BIT(adc_cfg.channel_id),
        .buffer = NULL,
        .buffer_size = 0,
        .resolution = 12,
    };
    
    HC4067 mux(mux_s0, mux_s1, mux_s2, mux_s3, mux_en);
    if (!mux.initialize()) {
        LOG_ERR("Failed to initialize HC4067 mux");
        return -EIO;
    }
    mux.set_enabled(true);
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -EIO;
    }
    if (!mux.configure_adc(adc_dev, adc_cfg.channel_id, adc_seq.resolution)) {
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

