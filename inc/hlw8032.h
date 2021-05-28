#ifndef __HLW8032_FPGA_UART_H__
#define __HLW8032_FPGA_UART_H__

#include "queue.h"

#define DATA_LEN (24)

typedef struct power_t {
   int16_t apparent_power;
   int16_t current;
   int16_t true_power;
   int16_t voltage;
} power_t;

QueueHandle_t xQueue;

power_t hlw8032_read();
void HLW8032DataTask(void *pvParameters);
power_t parse_data(uint8_t *data);

#endif /* __HLW8032_FPGA_UART_H__ */
