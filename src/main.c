#include "Fw_global_config.h"   // This defines application specific charactersitics

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "RtosTask.h"

/*    Include the generic headers required for QORC */
#include "eoss3_hal_gpio.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_uart.h"
#include "dbg_uart.h"

#include "fpga_loader.h"    // API for loading FPGA
#include "top_bit.h"        // FPGA bitstream to load into FPGA
#include "sensor_ssss.h"
#include "ssi_comms.h"
#include "micro_tick64.h"
#include "hlw8032.h"

extern void qf_hardwareSetup();
static void nvic_init(void);
static void init_fpga_uart(void);

static const fw_global_config_t fw_global_config_vars =
{
    .ssi_sensor_select_ssss = SSI_SENSOR_SELECT_SSSS,
    .sensor_ssss_livestream_enabled = SENSOR_SSSS_LIVESTREAM_ENABLED,
    .sensor_ssss_recog_enabled = SENSOR_SSSS_RECOG_ENABLED
};

int main(void)
{
    qf_hardwareSetup();
    nvic_init();
    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
    load_fpga(sizeof(axFPGABitStream), axFPGABitStream);

    // UART_ID_HW: application messages output;  default baud/parity
    // UART_ID_FPGA: HLW8032 data packets input; BAUD_4800/PARTY_EVEN
    // UART_ID_FPGA_UART1: debug message output; BAUD_115200/PARITY_NONE
    // Initilize FPGA UART2x 
    init_fpga_uart();

    sensor_ssss_block_processor();
    StartSimpleStreamingInterfaceTask();

    dbg_str( "\n\nGetting data from Atom Socket!!\n\n");

    xTaskCreate(HLW8032DataTask, "HLW8032Data", configMINIMAL_STACK_SIZE, NULL , 2, NULL );
    xTaskSet_uSecCount(1546300800ULL * 1000ULL * 1000ULL); // start at 2019-01-01 00:00:00 UTC time

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    dbg_str("\n");
      
    while(1);
}

static void nvic_init(void)
 {
    // To initialize system, this interrupt should be triggered at main.
    // So, we will set its priority just before calling vTaskStartScheduler(), not the time of enabling each irq.
    NVIC_SetPriority(Ffe0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SpiMs_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(CfgDma_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(Uart_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(FbMsg_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
 }    

//needed for startup_EOSS3b.s asm file
void SystemInit(void)
{

}

void init_fpga_uart()
{
    UartBaudRateType brate;
    UartHandler uartObj;
    memset( (void *)&(uartObj), 0, sizeof(uartObj) );

    uartObj.baud = BAUD_4800;
    uartObj.wl = WORDLEN_8B;
    uartObj.parity = PARITY_EVEN;
    uartObj.stop = STOPBITS_1;
    uartObj.mode = TX_RX_MODE;
    uartObj.hwCtrl = HW_FLOW_CTRL_DISABLE;
    uartObj.intrMode = UART_INTR_ENABLE;
    uartHandlerUpdate(UART_ID_FPGA, &uartObj);
    // wait
    for (volatile int i = 0; i != 4000000; i++) ;   

    uart_init(UART_ID_FPGA, NULL, NULL, &uartObj);

    uint32_t device_id = *(uint32_t *)FPGA_PERIPH_BASE ;

    if (device_id == 0xABCD0002)
    {
      uartObj.baud = BAUD_115200;
      uartObj.wl = WORDLEN_8B;
      uartObj.parity = PARITY_NONE;
      uartObj.stop = STOPBITS_1;
      uartObj.mode = TX_RX_MODE;
      uartObj.hwCtrl = HW_FLOW_CTRL_DISABLE;
      uartObj.intrMode = UART_INTR_ENABLE;
      uartHandlerUpdate(UART_ID_FPGA_UART1,&uartObj);

      uart_init(UART_ID_FPGA_UART1, NULL, NULL, &uartObj);
    }
}
