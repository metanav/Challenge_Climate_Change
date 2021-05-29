#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "eoss3_hal_uart.h"
#include "dbg_uart.h"
#include "hlw8032.h"
#include "task.h"

power_t hlw8032_read()
{
    power_t p_data;

    if (xQueueReceive(xQueue, &p_data, 0) != pdPASS) {
        dbg_str("hlw8032_read failed\r\n");
	p_data.valid = false;
    }

    return p_data;
}

void HLW8032DataTask(void *pvParameters)
{
    dbg_str("HLW8032DataTask started\r\n");
    int  rx_avail = 0;
    uint8_t data[DATA_LEN];

    xQueue = xQueueCreate(50, sizeof(power_t));

    memset(data, 0, DATA_LEN);

    for( ;; ) {
	// the data packet is being transmitted @ 4800 baud rate
	// and there is 50ms pause after each data packet
	// the delay value below is specific to the HLW8032 IC
        vTaskDelay(48);
	
	rx_avail = uart_rx_available(UART_ID_FPGA);

	// discard if data packet size is not 24 bytes 
	if (rx_avail != DATA_LEN) {
            int count = rx_avail;
            while (count > 0) {
		uart_rx(UART_ID_FPGA);
                count--;
            }
	    continue;
	}

	uart_rx_raw_buf(UART_ID_FPGA, data, DATA_LEN);

	// the 2nd byte must be 0x5A otherwise dicard the packet
	if (data[1] != 0x5A) {
	    rx_avail = uart_rx_available(UART_ID_FPGA);
	    uart_rx_raw_buf(UART_ID_FPGA, data, rx_avail);
	    continue;
	}

        uint8_t sum = 0;

	for (uint8_t i = 0; i < DATA_LEN; i++) {
	    // add up from 3rd byte (index=2) to 23rd byte (index=22) for checksum
	    if (i >= 2 && i <= DATA_LEN -2) {
                sum += data[i];
	    }
	    // print data
            //dbg_ch('0');
            //dbg_ch('x');
            //dbg_hex8(data[i]);
	    //if (i < DATA_LEN -1) {
            //    dbg_ch(' '); 
	    //} else {
            //    dbg_nl();
	    //}
	}

	// the 24th byte (index=23) is checksum
	
	if (sum == data[DATA_LEN-1]) {
	    //dbg_str("Checksum [PASS]\r\n");

	    power_t p_data = parse_data(data);

	    if (xQueueSendToBack(xQueue, &p_data, 0) != pdPASS) {
               dbg_str("no queue space\r\n");
            }
	} else {
	    dbg_str("Checksum [FAIL]\r\n");
	}
    }
}

power_t parse_data(uint8_t *data)
{
    uint32_t volt_R1 = 1880000;
    uint32_t volt_R2 = 1000;
    float current_RF = 0.001;
    float VF = volt_R1 / volt_R2 / 1000.0;
    float CF = 1.0 / (current_RF * 1000.0);

    uint32_t volt_param    = ((uint32_t)data[2]  <<16) | ((uint32_t)data[3] <<8)  | data[4];
    uint32_t volt_data     = ((uint32_t)data[5]  <<16) | ((uint32_t)data[6] <<8)  | data[7];
    uint32_t current_param = ((uint32_t)data[8]  <<16) | ((uint32_t)data[9] <<8)  | data[10];
    uint32_t current_data  = ((uint32_t)data[11] <<16) | ((uint32_t)data[12] <<8) | data[13];
    uint32_t power_param   = ((uint32_t)data[14] <<16) | ((uint32_t)data[15] <<8) | data[16];
    uint32_t power_data    = ((uint32_t)data[17] <<16) | ((uint32_t)data[18] <<8) | data[19];

    // uint32_t PF_data = 1;
    // uint16_t PF = ((uint32_t)data[21] <<8) | data[22];
    // if(bitRead(data[20], 7) == 1) {
    //     PFData++;
    // }

    float current        = ((float)current_param / (float)current_data) * CF - 0.06f;
    float voltage        = ((float)volt_param / (float)volt_data) * VF;
    float apparent_power = current * voltage;
    float true_power     = ((float)power_param / (float)power_data) * VF * CF;

    if ( current < 0.0f ) {
      current = 0.0f;
    }

    dbg_str_fraction("Current", current_param * CF - 0.06 * current_data, current_data);
    dbg_str_fraction("Voltage", volt_param * VF, volt_data);
    dbg_str_fraction("Apparent Power", apparent_power * 1000, 1000);
    dbg_str_fraction("True Power", power_param  * VF * CF, power_data);

    //char values[50];
    //sprintf(
    //    values,
    //    "AP=%0.2fVA, C=%0.4fA, TP=%0.2fW, V=%0.2fV\n",
    //    apparent_power, current, true_power, voltage
    //);
    //dbg_str(values);

    power_t p_data = {
	.apparent_power = (int16_t) (apparent_power * 100),
	.current = (int16_t) (current * 1000), // mA
	.true_power = (int16_t)  (true_power * 100),
	.voltage = (int16_t) (voltage * 10),
	.valid = true
    };

    return p_data;
}
