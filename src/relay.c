#include <stdint.h>
#include <stdbool.h>
#include "Fw_global_config.h"
#include "dbg_uart.h"
#include "eoss3_hal_gpio.h"
#include "relay.h"

void RelaySwitchTask(void *pvParameters)
{
    dbg_str("RelaySwitchTask started\r\n");
    bool state = false;
    bool prev_state = state;
    uint8_t ucVal;

    HAL_GPIO_Write(RELAY_PIN, 0);

    for( ;; ) {
        HAL_GPIO_Read(BUTTON_PIN, &ucVal);
	// on button pressed
        if (!ucVal) {
	    state = !state;
	    dbg_str("Button Pressed\r\n");
	    vTaskDelay(300);
        if (prev_state != state) {
            if (state) {
                HAL_GPIO_Write(RELAY_PIN, 1);
                dbg_str("Relay ON\r\n");
            } else {
                HAL_GPIO_Write(RELAY_PIN, 0);
                dbg_str("Relay OFF\r\n");
            }
            prev_state = state;
        }
	}
	
	vTaskDelay(100);
	
    }
}

